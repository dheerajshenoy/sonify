#pragma once

#include <cstdio>

FILE *
ffmpeg_audio_video(int w, int h, unsigned int fps, const char *audioPath,
                   const char *outPath) noexcept;
