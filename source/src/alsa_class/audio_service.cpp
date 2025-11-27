#include "audio_service.h"
#include "logger.h"

void AudioService::Initialize(){
    /* Setup the audio codec */
    // opus_decoder_ = std::make_unique<OpusDecoderWrapper>(codec->output_sample_rate(), 1, OPUS_FRAME_DURATION_MS);
    // opus_encoder_ = std::make_unique<OpusEncoderWrapper>(16000, 1, OPUS_FRAME_DURATION_MS);
    // std::unique_ptr<AudioTask> task(new AudioTask());
    // opus_encoder_ = std::unique_ptr<OpusEncoderWrapper>(new OpusEncoderWrapper(16000, 1, OPUS_FRAME_DURATION_MS));
    opus_decoder_.reset(new OpusDecoderWrapper(16000, 1, OPUS_FRAME_DURATION_MS));  // play->output_sample_rate(), 
    opus_encoder_.reset(new OpusEncoderWrapper(16000, 1, OPUS_FRAME_DURATION_MS));
    opus_encoder_->SetComplexity(0);
    Logger::info("AudioService::Initialize finish");
}

void AudioService::Start(){
    service_stopped_ = false;
    // 启动音频采集线程
    input_thread = std::thread(&AudioService::AudioInputTask, this);
    input_thread.detach();
    
    // 启动处理线程
    output_thread = std::thread(&AudioService::AudioOutputTask, this);
    output_thread.detach();

    opus_thread = std::thread(&AudioService::OpusCodecTask, this);
    opus_thread.detach();

    std::cout << "### AudioService started... " << std::endl;
}

void AudioService::Stop(){

}


bool AudioService::ReadAudioData(std::vector<int16_t>& data, int sample_rate, int samples) {
    // Logger::info("ReadAudioData ... 001");
    if (capture.input_sample_rate() != sample_rate) {
        // data.resize(samples * capture->input_sample_rate() / sample_rate);
        // if (!capture->InputData(data)) {
        //     return false;
        // }
        // if (capture->input_channels() == 2) {
        //     auto mic_channel = std::vector<int16_t>(data.size() / 2);
        //     auto reference_channel = std::vector<int16_t>(data.size() / 2);
        //     for (size_t i = 0, j = 0; i < mic_channel.size(); ++i, j += 2) {
        //         mic_channel[i] = data[j];
        //         reference_channel[i] = data[j + 1];
        //     }
        //     auto resampled_mic = std::vector<int16_t>(input_resampler_.GetOutputSamples(mic_channel.size()));
        //     auto resampled_reference = std::vector<int16_t>(reference_resampler_.GetOutputSamples(reference_channel.size()));
        //     input_resampler_.Process(mic_channel.data(), mic_channel.size(), resampled_mic.data());
        //     reference_resampler_.Process(reference_channel.data(), reference_channel.size(), resampled_reference.data());
        //     data.resize(resampled_mic.size() + resampled_reference.size());
        //     for (size_t i = 0, j = 0; i < resampled_mic.size(); ++i, j += 2) {
        //         data[j] = resampled_mic[i];
        //         data[j + 1] = resampled_reference[i];
        //     }
        // } else {
        //     auto resampled = std::vector<int16_t>(input_resampler_.GetOutputSamples(data.size()));
        //     input_resampler_.Process(data.data(), data.size(), resampled.data());
        //     data = std::move(resampled);
        // }
        
    } else {
        // Logger::info("ReadAudioData ... 003,  ");
        data.resize(samples);
        if (!capture.InputData(data)) {
            return false;
        }
        else{
            // Logger::info("ReadAudioData success ... 005");
        }
    }
    return true;
}

void AudioService::AudioInputTask(){
    while (true) {
        if (service_stopped_) {
            break;
        }
        std::vector<int16_t> data;
        int samples = OPUS_FRAME_DURATION_MS * 16000 / 1000;   //960
        if (ReadAudioData(data, 16000, samples)) {
            // If input channels is 2, we need to fetch the left channel data
            if (capture.input_channels() == 2) {
                auto mono_data = std::vector<int16_t>(data.size() / 2);
                for (size_t i = 0, j = 0; i < mono_data.size(); ++i, j += 2) {
                    mono_data[i] = data[j];
                }
                data = std::move(mono_data);
            }  // kAudioTaskTypeEncodeToSendQueue   kAudioTaskTypeEncodeToTestingQueue
            PushTaskToEncodeQueue(kAudioTaskTypeEncodeToSendQueue, std::move(data));
            continue;
        }
        break;
    }
}

void AudioService::AudioOutputTask(){
    Logger::info("AudioOutputTask ... 001");
    while (true) {
        std::unique_lock<std::mutex> lock(audio_queue_mutex_);
        audio_queue_cv_.wait(lock, [this]() { return !audio_playback_queue_.empty() || service_stopped_; });
        if (service_stopped_) {
            break;
        }

        auto task = std::move(audio_playback_queue_.front());
        audio_playback_queue_.pop_front();
        audio_queue_cv_.notify_all();
        lock.unlock();

        // if (!codec_->output_enabled()) {
        //     codec_->EnableOutput(true);
        //     esp_timer_start_periodic(audio_power_timer_, AUDIO_POWER_CHECK_INTERVAL_MS * 1000);
        // }
        int stero_size = task->pcm.size() * 2;
        std::vector<int16_t> stero_data(stero_size);
        for (int i =0; i< task->pcm.size(); i++)
        {
            stero_data[2 * i] = task->pcm[i];
            stero_data[2 * i + 1] = task->pcm[i];
        }
        play.OutputData(stero_data);
        // play.OutputData(task->pcm);

        /* Update the last output time */
        last_output_time_ = std::chrono::steady_clock::now();
        debug_statistics_.playback_count++;

    #if CONFIG_USE_SERVER_AEC
        /* Record the timestamp for server AEC */
        if (task->timestamp > 0) {
            lock.lock();
            timestamp_queue_.push_back(task->timestamp);
        }
    #endif
    }
}

void AudioService::OpusCodecTask() {
    Logger::info("OpusCodecTask ... 001");
    while (true) {
        std::unique_lock<std::mutex> lock(audio_queue_mutex_);
        audio_queue_cv_.wait(lock, [this]() {
            return service_stopped_ ||
                (!audio_encode_queue_.empty() && audio_send_queue_.size() < MAX_SEND_PACKETS_IN_QUEUE) ||
                (!audio_decode_queue_.empty() && audio_playback_queue_.size() < MAX_PLAYBACK_TASKS_IN_QUEUE);
        });
        if (service_stopped_) {
            break;
        }

        /* Decode the audio from decode queue */
        if (!audio_decode_queue_.empty() && audio_playback_queue_.size() < MAX_PLAYBACK_TASKS_IN_QUEUE) {
            auto packet = std::move(audio_decode_queue_.front());
            audio_decode_queue_.pop_front();
            audio_queue_cv_.notify_all();
            lock.unlock();

            // auto task = std::make_unique<AudioTask>();
            std::unique_ptr<AudioTask> task(new AudioTask());
            task->type = kAudioTaskTypeDecodeToPlaybackQueue;
            task->timestamp = packet->timestamp;

            // SetDecodeSampleRate(packet->sample_rate, packet->frame_duration);
            if (opus_decoder_->Decode(std::move(packet->payload), task->pcm)) {
                // Resample if the sample rate is different
                if (opus_decoder_->sample_rate() != play.output_sample_rate()) {
                    // int target_size = output_resampler_.GetOutputSamples(task->pcm.size());
                    // std::vector<int16_t> resampled(target_size);
                    // output_resampler_.Process(task->pcm.data(), task->pcm.size(), resampled.data());
                    // task->pcm = std::move(resampled);
                }

                lock.lock();
                audio_playback_queue_.push_back(std::move(task));
                audio_queue_cv_.notify_all();
            } else {
                // ESP_LOGE(TAG, "Failed to decode audio");
                lock.lock();
            }
            debug_statistics_.decode_count++;
        }
        
        /* Encode the audio to send queue */
        if (!audio_encode_queue_.empty() && audio_send_queue_.size() < MAX_SEND_PACKETS_IN_QUEUE) {
            auto task = std::move(audio_encode_queue_.front());
            audio_encode_queue_.pop_front();
            audio_queue_cv_.notify_all();
            lock.unlock();

            // auto packet = std::make_unique<AudioStreamPacket>(); //
            std::unique_ptr<AudioStreamPacket> packet(new AudioStreamPacket());
            packet->frame_duration = OPUS_FRAME_DURATION_MS;
            packet->sample_rate = 16000;
            packet->timestamp = task->timestamp;
            if (!opus_encoder_->Encode(std::move(task->pcm), packet->payload)) {
                // ESP_LOGE(TAG, "Failed to encode audio");
                continue;
            }

            if (task->type == kAudioTaskTypeEncodeToSendQueue) {
                {
                    std::lock_guard<std::mutex> lock(audio_queue_mutex_);
                    audio_send_queue_.push_back(std::move(packet));
                }
                // if (callbacks_.on_send_queue_available) {
                //     callbacks_.on_send_queue_available();
                // }
            } else if (task->type == kAudioTaskTypeEncodeToTestingQueue) {  
                std::lock_guard<std::mutex> lock(audio_queue_mutex_);
                audio_testing_queue_.push_back(std::move(packet));
            }
            debug_statistics_.encode_count++;
            lock.lock();
        }
    }
}


void AudioService::PushTaskToEncodeQueue(AudioTaskType type, std::vector<int16_t>&& pcm) {
    // auto task = std::make_unique<AudioTask>();
    std::unique_ptr<AudioTask> task(new AudioTask());
    task->type = type;
    task->pcm = std::move(pcm);
    
    /* Push the task to the encode queue */
    std::unique_lock<std::mutex> lock(audio_queue_mutex_);

    /* If the task is to send queue, we need to set the timestamp */
    if (type == kAudioTaskTypeEncodeToSendQueue && !timestamp_queue_.empty()) {
        if (timestamp_queue_.size() <= MAX_TIMESTAMPS_IN_QUEUE) {
            task->timestamp = timestamp_queue_.front();
        } else {
            // ESP_LOGW(TAG, "Timestamp queue (%u) is full, dropping timestamp", timestamp_queue_.size());
        }
        timestamp_queue_.pop_front();
    }

    audio_queue_cv_.wait(lock, [this]() { return audio_encode_queue_.size() < MAX_ENCODE_TASKS_IN_QUEUE; });
    audio_encode_queue_.push_back(std::move(task));
    audio_queue_cv_.notify_all();
}


bool AudioService::PushPacketToDecodeQueue(std::unique_ptr<AudioStreamPacket> packet, bool wait) {
    std::unique_lock<std::mutex> lock(audio_queue_mutex_);
    if (audio_decode_queue_.size() >= MAX_DECODE_PACKETS_IN_QUEUE) {
        if (wait) {
            audio_queue_cv_.wait(lock, [this]() { return audio_decode_queue_.size() < MAX_DECODE_PACKETS_IN_QUEUE; });
        } else {
            return false;
        }
    }
    audio_decode_queue_.push_back(std::move(packet));
    audio_queue_cv_.notify_all();
    return true;
}

std::unique_ptr<AudioStreamPacket> AudioService::PopPacketFromSendQueue() {
    std::lock_guard<std::mutex> lock(audio_queue_mutex_);
    if (audio_send_queue_.empty()) {
        return nullptr;
    }
    auto packet = std::move(audio_send_queue_.front());
    // Logger::info("PopPacketFromSendQueue ... 002 packet->payload.size() = %d", packet->payload.size() );
    audio_send_queue_.pop_front();
    audio_queue_cv_.notify_all();
    return packet;
}


void AudioService::UpdateOutputTimestamp() {
    last_output_time_ = std::chrono::steady_clock::now();
}

