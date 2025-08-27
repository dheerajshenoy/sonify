#include "Sonify.hpp"
#include "argparse.hpp"

void
init_args(argparse::ArgumentParser &args)
{
    args.add_argument("--samplerate", "-s")
        .scan<'g', float>()
        .default_value(44100.0f)
        .help("Samplerate of the audio");

    // args.add_argument("-s").scan<'i', int>().help("Samplerate of the audio");

    args.add_argument("--channels", "-c")
        .scan<'i', int>()
        .default_value(2)
        .help("No. of channels to be used");

    // args.add_argument("-c").scan<'i', int>().help("No. of channels to be
    // used");

    args.add_argument("--output", "-o").help("Output WAV file name");

    args.add_argument("--traversal", "-t")
        .scan<'i', int>()
        .default_value(0)
        .help("ID of the traversal to be used");

    // args.add_argument("-t").scan<'i', int>().help(
    //     "ID of the traversal to be used");

    args.add_argument("--background", "-b")
        .scan<'i', unsigned int>()
        .help("Background color");

    args.add_argument("--pixelmap", "-p")
        .default_value("Intensity")
        .help("Name of the pixel mapping to be used");

    args.add_argument("--no-spectrum")
        .flag()
        .help("Do not display FFT spectrum");

    args.add_argument("--headless").flag().help("Run an headless instance");

    args.add_argument("--loop").flag().help("Enable audio looping");

    args.add_argument("--silent").flag().help("Silence INFO/WARNING messages");

    args.add_argument("--dps").scan<'g', float>().default_value(0.05f).help(
        "Durations per sample");

    // args.add_argument("--in-fmin")
    //     .scan<'i', unsigned int>()
    //     .help("Input minimum frequency");
    //
    // args.add_argument("--in-fmax")
    //     .scan<'i', unsigned int>()
    //     .help("Input maximum frequency");

    args.add_argument("--fmin").scan<'g', float>().default_value(0.0f).help(
        "Output minimum frequency");

    args.add_argument("--fmax").scan<'g', float>().default_value(20000.0f).help(
        "Output maximum frequency");

    // args.add_argument("-n").help("Disable GUI, run headless (just audio)");

    args.add_argument("--fps").scan<'i', int>().help("FPS of the GUI");

    args.add_argument("--input", "-i").help("Input file");
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
