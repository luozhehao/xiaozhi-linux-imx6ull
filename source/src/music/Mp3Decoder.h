#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include <vector>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <mad.h>

class Mp3Decoder
{
public:
    Mp3Decoder();
    ~Mp3Decoder();

    // 输入一块MP3数据，输出PCM数据（16bit）
    // 返回输出PCM样本数（不是字节数）
    size_t decode(const unsigned char* input, size_t input_size,
                  std::vector<short>& pcm_out);

    void reset();
    size_t skip_id3v2(std::ifstream &mp3);
    void setInfile(std::string str) { infile = str; }
    void setOutfile(std::string str) { outfile = str; }
    std::string getInfile() { return infile; }
    std::string getOutfile() { return outfile; }
    int getMp3Channels() { return mp3_channels; }
    int getMp3Samplerate() { return mp3_samplerate; }

private:
    inline short madScale(mad_fixed_t sample);

private:
    struct mad_stream  m_stream;
    struct mad_frame   m_frame;
    struct mad_synth   m_synth;
    int mp3_channels;
    int mp3_samplerate;
    int mp3_sample_num;

    std::vector<unsigned char> m_buffer;
    size_t m_remaining;

    std::string infile;
    std::string outfile;
};

#endif