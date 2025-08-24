#include "Sonify.hpp"
#include "argparse.hpp"

void
init_args(argparse::ArgumentParser &args)
{
    args.add_argument("--samplerate")
        .scan<'i', int>()
        .help("Samplerate of the audio");

    // args.add_argument("-s").scan<'i', int>().help("Samplerate of the audio");

    args.add_argument("--channels")
        .scan<'i', int>()
        .help("No. of channels to be used");

    // args.add_argument("-c").scan<'i', int>().help("No. of channels to be
    // used");

    args.add_argument("--output").help("Output WAV file name");

    args.add_argument("--traversal")
        .scan<'i', int>()
        .help("ID of the traversal to be used");

    // args.add_argument("-t").scan<'i', int>().help(
    //     "ID of the traversal to be used");

    args.add_argument("--background")
        .scan<'i', unsigned int>()
        .help("Background color");

    args.add_argument("--pixelmap")
        .help("Name of the pixel mapping to be used");

    // args.add_argument("-p").help("Name of the pixel mapping to be used");

    args.add_argument("--no-gui")
        .help("Disable GUI, run headless (just audio)");

    // args.add_argument("-n").help("Disable GUI, run headless (just audio)");

    args.add_argument("--fps").scan<'i', int>().help("FPS of the GUI");

    args.add_argument("FILE").remaining().help("Input file");
}

int
main(int argc, char **argv)
{
    argparse::ArgumentParser program("Sonify", __SONIFY_VERSION);
    init_args(program);
    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what();
        return 1;
    }

    Sonify s(program);
    return 0;
}
