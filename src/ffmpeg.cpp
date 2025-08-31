#include "ffmpeg.hpp"

#include <cstdio>
#include <format>

FILE *
ffmpeg_audio_video(int w, int h, unsigned int fps, const char *audioPath,
                   const char *outPath) noexcept
{
    std::string cmd;
    cmd = std::format("ffmpeg -loglevel verbose -y "
                      "-f rawvideo -pix_fmt rgba -s {}x{} -r {} -i - "
                      "-i '{}' "
                      "-c:v libx264 -pix_fmt yuv420p "
                      "-c:a aac "
                      "'{}'",
                      w, h, fps, audioPath, outPath);

    FILE *pipe = popen(cmd.c_str(), "w");

    if (!pipe)
    {
        std::perror("popen ffmpeg");
        return nullptr;
    }

    setvbuf(pipe, nullptr, _IONBF, 0);
    return pipe;
}
