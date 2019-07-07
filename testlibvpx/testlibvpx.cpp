// testlibvpx.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "OpusVorbisDecoder.hpp"
#include "VPXDecoder.hpp"
#include <mkvparser/mkvparser.h>
#include <stdio.h>
#include <fstream>
#include <libyuv.h>
#include <libpng/png.h>
#include <string>
#include <sstream>

using namespace libyuv;


int make_png(const std::string& const filename, const VPXDecoder::Image& const image, const uint8_t* const argb);

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
	{
		std::cout << "webmtopng.exe *.webm" << std::endl;
		return -1;
	}


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



					const int w = image.w;
					const int h = image.h;
					int size = w * h * 4;
					uint8_t* desc = new uint8_t[size]{ 0 };
					if (image.planes[3] == nullptr)
					{
						if (I420ToARGB(image.planes[0], image.linesize[0]
							, image.planes[1], image.linesize[1]
							, image.planes[2], image.linesize[2]
							, desc, w * 4
							, w, h) == 0)
						{

							/*auto file = std::ofstream("a.raw", std::ios::binary);
							file.seekp(0, file.beg);
							file.write(reinterpret_cast<const char*>(desc), size);
							file.close();*/

							std::string filename = argv[1];
							filename = filename.replace(filename.end() - 5, filename.end(), "");
							filename.append("_");
							std::stringstream ss;
							ss << filename << ++i << ".png";
							ss >> filename;



							if (make_png(filename, image, desc))
							{
								std::cout << "error write png" << std::endl;
							}
						}
						else
						{
							std::cout << "error I420ToARGB -1" << std::endl;
						}

					}
					else
					{
						if (I420AlphaToARGB(image.planes[0], image.linesize[0]
							, image.planes[1], image.linesize[1]
							, image.planes[2], image.linesize[2]
							, image.planes[3], image.linesize[3]
							, desc, w * 4
							, w, h, 0) == 0)
						{

							/*	auto file = std::ofstream("a.raw", std::ios::binary);
								file.seekp(0, file.beg);
								file.write(reinterpret_cast<const char*>(desc), size);
								file.close();*/

							std::string filename = argv[1];
							filename = filename.replace(filename.end() - 5, filename.end(), "");
							filename.append("_");
							std::stringstream ss;
							ss << filename << ++i << ".png";
							ss >> filename;

							if (make_png(filename, image, desc))
								std::cout << "error write png" << std::endl;					

						}
						else
						{
							std::cout << "error I420ToARGB -1" << std::endl;
						}

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

	std::cout << "close." << std::endl;


	return 0;
}


int make_png(const std::string& const filename, const VPXDecoder::Image& const image, const uint8_t* const argb) {

	png_structp png_ptr;
	png_infop 	info_ptr;
	const png_byte   color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	png_bytep* row_pointers;
	FILE* fp = fopen(filename.c_str(), "wb");
	if (NULL == fp) {
		printf("open failed ...2\n");		return -1;
	}


	//1: 初始化libpng结构体  

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if (!png_ptr) {
		printf("png_create_write_struct failed ...\n");
		return -1;
	}

	//2: 初始化png_infop结构体 ， 

	//此结构体包含了图像的各种信息如尺寸，像素位深, 颜色类型等等

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("png_create_info_struct failed ...\n");
		return -1;
	}
	//3: 设置错误返回点

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("error during init_io ...\n");
		return -1;
	}

	//4:绑定文件IO到Png结构体
	png_init_io(png_ptr, fp);

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("error during init_io ...\n");
		return -1;
	}

	//5：设置以及写入头部信息到Png文件
	png_set_IHDR(png_ptr, info_ptr, image.w, image.h, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("error during init_io ...\n");
		return -1;
	}


	int i, j, pos = 0;
	int temp = (4 * image.w);
	row_pointers = (png_bytep*)malloc(image.h * sizeof(png_bytep));

	for (i = 0; i < image.h; i++) {
		row_pointers[i] = (png_bytep)malloc(temp * sizeof(unsigned char));
		for (j = 0; j < temp; j += 4) {
			row_pointers[i][j + 2] = argb[pos++];
			row_pointers[i][j + 1] = argb[pos++];
			row_pointers[i][j + 0] = argb[pos++];
			row_pointers[i][j + 3] = argb[pos++];
		}
	}

	png_write_image(png_ptr, (png_bytepp)row_pointers);

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("error during init_io ...\n");
		return -1;
	}

	//7: 写入尾部信息

	png_write_end(png_ptr, NULL);

	//8:释放内存 ,销毁png结构体

	for (i = 0; i < image.h; i++)
		free(row_pointers[i]);

	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	std::cout << filename << " success" << std::endl;
	return 0;
}

