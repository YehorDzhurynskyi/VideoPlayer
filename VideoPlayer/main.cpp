#include "pch.h"
#include "SDL.h"
#include "SDL_main.h"

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

constexpr const epiChar* url = "video.mp4";

#undef main
int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    AVFormatContext* ctx = nullptr;
    if (avformat_open_input(&ctx, url, nullptr, nullptr) != 0)
    {
        return -1;
    }

    if (avformat_find_stream_info(ctx, nullptr))
    {
        return -1;
    }

    av_dump_format(ctx, 0, url, 0);

    epiS32 videoStreamIndex = -1;
    for (epiS32 i = 0; i < ctx->nb_streams; ++i)
    {
        if (ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        return -1;
    }

    AVCodecContext* origVideoCodecCtx = ctx->streams[videoStreamIndex]->codec;
    AVCodec* codec = avcodec_find_decoder(origVideoCodecCtx->codec_id);
    if (codec == nullptr)
    {
        return -1;
    }

    AVCodecContext* videoCodecCtx = avcodec_alloc_context3(codec);
    if (avcodec_copy_context(videoCodecCtx, origVideoCodecCtx) != 0)
    {
        return -1;
    }

    if (avcodec_open2(videoCodecCtx, codec, nullptr) < 0)
    {
        return -1;
    }

    AVFrame* rgbFrame = av_frame_alloc();
    av_image_alloc(rgbFrame->data, rgbFrame->linesize, videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24, 1);

    SDL_Window* win = SDL_CreateWindow("Video Player",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       videoCodecCtx->width,
                                       videoCodecCtx->height,
                                       SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGB24,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             videoCodecCtx->width,
                                             videoCodecCtx->height);

    SwsContext* swsCtx = sws_getContext(videoCodecCtx->width,
                                        videoCodecCtx->height,
                                        videoCodecCtx->pix_fmt,
                                        videoCodecCtx->width,
                                        videoCodecCtx->height,
                                        AV_PIX_FMT_RGB24,
                                        SWS_BILINEAR,
                                        nullptr,
                                        nullptr,
                                        nullptr);

    AVFrame* rawFrame = av_frame_alloc();
    AVPacket packet;
    while (av_read_frame(ctx, &packet) >= 0)
    {
        if (packet.stream_index == videoStreamIndex)
        {
            avcodec_send_packet(videoCodecCtx, &packet);

            while (!avcodec_receive_frame(videoCodecCtx, rawFrame))
            {
                sws_scale(swsCtx,
                          rawFrame->data,
                          rawFrame->linesize,
                          0,
                          videoCodecCtx->height,
                          rgbFrame->data,
                          rgbFrame->linesize);

                SDL_UpdateTexture(texture, nullptr, rgbFrame->data[0], rgbFrame->linesize[0]);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
            }
        }

        av_packet_unref(&packet);

        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            SDL_Quit();
            exit(0);
            break;
        default:
            break;
        }

        SDL_Delay(33);
    }

    av_freep(rgbFrame->data);
    av_free(rgbFrame);
    av_free(rawFrame);

    avcodec_close(videoCodecCtx);
    avcodec_close(origVideoCodecCtx);

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
