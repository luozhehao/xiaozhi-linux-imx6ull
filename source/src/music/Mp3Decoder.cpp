#include "Mp3Decoder.h"
#include <iostream>

static const size_t MP3_INPUT_BUF = 8192;

Mp3Decoder::Mp3Decoder()
    : m_buffer(MP3_INPUT_BUF + MAD_BUFFER_GUARD),
      m_remaining(0)
{
    mad_stream_init(&m_stream);
    mad_frame_init(&m_frame);
    mad_synth_init(&m_synth);
}

Mp3Decoder::~Mp3Decoder()
{
    mad_synth_finish(&m_synth);
    mad_frame_finish(&m_frame);
    mad_stream_finish(&m_stream);
}

void Mp3Decoder::reset()
{
    mad_stream_finish(&m_stream);
    mad_frame_finish(&m_frame);
    mad_synth_finish(&m_synth);

    mad_stream_init(&m_stream);
    mad_frame_init(&m_frame);
    mad_synth_init(&m_synth);

    m_remaining = 0;
}

// inline short Mp3Decoder::madScale(mad_fixed_t sample)
// {
//     // sample += (1L << (MAD_F_FRACBITS - 16));
//     // if (sample > MAD_F_ONE - 1)
//     //     sample = MAD_F_ONE - 1;
//     // else if (sample < -MAD_F_ONE)
//     //     sample = -MAD_F_ONE;
//     // return (short)(sample >> (MAD_F_FRACBITS - 16));
// }

/*
转换24bit pcm为16bit pcm
 */ // // https://blog.csdn.net/A694543965/article/details/79711299
// static inline
// signed int scale(mad_fixed_t sample)
inline short Mp3Decoder::madScale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));
 
  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;
 
  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}


size_t Mp3Decoder::decode(const unsigned char* input, size_t input_size,
                          std::vector<short>& pcm_out)
{
    pcm_out.clear();

    // 将未处理的残留移动到前面
    if (m_remaining > 0) {
        memmove(m_buffer.data(), m_stream.next_frame, m_remaining);
    }

    // 填充新数据
    if (input_size > 0) {
        memcpy(m_buffer.data() + m_remaining, input, input_size);
    }

    size_t buf_size = m_remaining + input_size;

    // 如果到达流末尾，可添加 MAD_BUFFER_GUARD（optional）
    if (input_size == 0) {
        memset(m_buffer.data() + buf_size, 0, MAD_BUFFER_GUARD);
        buf_size += MAD_BUFFER_GUARD;
    }

    // 喂给 mad
    mad_stream_buffer(&m_stream, m_buffer.data(), buf_size);

    // 解码循环
    while (true) {
        if (mad_frame_decode(&m_frame, &m_stream) == 0) {
            mad_synth_frame(&m_synth, &m_frame);
            // std::cout << "m_synth.pcm.length = " << m_synth.pcm.length 
            //         << "\n m_synth.pcm.channels = " << m_synth.pcm.channels 
            //          << std::endl;
            mp3_channels = m_synth.pcm.channels;
            mp3_samplerate = m_synth.pcm.samplerate;
            mp3_sample_num = m_synth.pcm.length;

            // 输出PCM（左/右声道）
            for (unsigned int i = 0; i < m_synth.pcm.length; i++) {
                if (m_synth.pcm.channels == 1) {
                    short left = madScale(m_synth.pcm.samples[0][i]);
                    pcm_out.push_back(left);
                    pcm_out.push_back(left);
                }

                if (m_synth.pcm.channels == 2) {
                    short left = madScale(m_synth.pcm.samples[0][i]);
                    pcm_out.push_back(left);
                    short right = madScale(m_synth.pcm.samples[1][i]);
                    pcm_out.push_back(right);
                }
            }

        } else {
            if (m_stream.error == MAD_ERROR_BUFLEN) {
                break; // 需要更多数据
            } else {
                // 跳过坏字节（libmad恢复机制）
                mad_stream_skip(&m_stream, 1);
            }
        }
    }

    // 计算剩余数据
    m_remaining = m_stream.bufend - m_stream.next_frame;

    return pcm_out.size();  // 返回样本数
}


size_t Mp3Decoder::skip_id3v2(std::ifstream &mp3)
{
    unsigned char hdr[10];
    mp3.read((char*)hdr, 10);
    if (mp3.gcount() < 10)
        return 0; // too small, no ID3

    if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3')
    {
        size_t size =
            ((hdr[6] & 0x7F) << 21) |
            ((hdr[7] & 0x7F) << 14) |
            ((hdr[8] & 0x7F) << 7)  |
            (hdr[9] & 0x7F);

        size_t skip_bytes = size + 10;
        mp3.seekg(skip_bytes, std::ios::beg);

        std::cout << "Skip ID3v2 tag: " << skip_bytes << " bytes\n";
        return skip_bytes;
    }

    // No ID3v2 → rewind
    mp3.seekg(0, std::ios::beg);
    return 0;
}