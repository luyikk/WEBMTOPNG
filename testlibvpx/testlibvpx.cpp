// testlibvpx.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "OpusVorbisDecoder.hpp"
#include "VPXDecoder.hpp"
#include <mkvparser/mkvparser.h>
#include <stdio.h>
#include <fstream>
#include <libyuv.h>
using namespace libyuv;


class MkvReader : public mkvparser::IMkvReader
{
public:
	MkvReader(const char* filePath) :
		m_file(fopen(filePath, "rb"))
	{}
	~MkvReader()
	{
		if (m_file)
			fclose(m_file);
	}

	int Read(long long pos, long len, unsigned char* buf)
	{
		if (!m_file)
			return -1;
		fseek(m_file, pos, SEEK_SET);
		const size_t size = fread(buf, 1, len, m_file);
		if (size < size_t(len))
			return -1;
		return 0;
	}
	int Length(long long* total, long long* available)
	{
		if (!m_file)
			return -1;
		const off_t pos = ftell(m_file);
		fseek(m_file, 0, SEEK_END);
		if (total)
			* total = ftell(m_file);
		if (available)
			* available = ftell(m_file);
		fseek(m_file, pos, SEEK_SET);
		return 0;
	}

private:
	FILE* m_file;
};

int main(int argc, const char** argv)
{
	if (argc != 2)
		return -1;



	WebMFrame videoFrame, audioFrame;

	WebMDemuxer demuxer(new MkvReader(argv[1]));
	if (demuxer.isOpen())
	{
		VPXDecoder videoDec(demuxer, 8);
		OpusVorbisDecoder audioDec(demuxer);

		WebMFrame videoFrame, audioFrame;

		VPXDecoder::Image image;
		short* pcm = audioDec.isOpen() ? new short[audioDec.getBufferSamples() * demuxer.getChannels()] : NULL;

		fprintf(stderr, "Length: %f\n", demuxer.getLength());
	
		int i = 0;
		while (demuxer.readFrame(&videoFrame, &audioFrame))
		{
			if (videoDec.isOpen() && videoFrame.isValid())
			{
				if (!videoDec.decode(videoFrame))
				{
					fprintf(stderr, "Video decode error\n");
					break;
				}
			
				while (videoDec.getImage(image) == VPXDecoder::NO_ERROR)
				{
					//auto file = std::fopen("o.yuv", "w");
					//for (int p = 0; p < 3; ++p)
					//{
					//	
					//	const int w = image.getWidth(p);
					//	const int h = image.getHeight(p);
					//	int offset = 0;

					//	for (int y = 0; y < h; ++y)
					//	{
					//		//fwrite(image.planes[p] + offset, 1, w, file);

					//		offset += image.linesize[p];
					//	}

					//
					//}
					//std::fclose(file);

					const int w = image.w;
					const int h = image.h;
					int size = w * h * 4;
					uint8_t * desc = new uint8_t[size]{ 0 };

				  if (I420ToARGB(image.planes[0], image.linesize[0]
						, image.planes[1], image.linesize[1]
						, image.planes[2], image.linesize[2]	
						, desc, w * 4
						, w, h) == 0)
					{

						auto file = std::ofstream("a.raw",std::ios::binary);
						file.seekp(0,file.beg);
						file.write(reinterpret_cast<const char*>(desc), size);						
						file.close();			

					}
					else
					{
						std::cout << "error I420ToARGB -1" << std::endl;
					}		
	
					delete[] desc;
					
				}
			}
			if (audioDec.isOpen() && audioFrame.isValid())
			{
				int numOutSamples;
				if (!audioDec.getPCMS16(audioFrame, pcm, numOutSamples))
				{
					fprintf(stderr, "Audio decode error\n");
					break;
				}
				// 				fwrite(pcm, 1, numOutSamples * demuxer.getChannels() * sizeof(short), stdout);
			}
		}

		delete[] pcm;
	
	}


	return 0;
}
