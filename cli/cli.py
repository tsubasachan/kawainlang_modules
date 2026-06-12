import sys


def _program_args():
    argv = sys.argv
    if len(argv) >= 2 and str(argv[1]).endswith(".🎀"):
        return argv[2:]
    return argv[1:]


def kl_cli_argc():
    return len(_program_args())


def kl_cli_arg(index):
    args = _program_args()
    index = int(index)
    if index < 0 or index >= len(args):
        return ""
    return args[index]
