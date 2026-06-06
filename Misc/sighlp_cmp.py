#!/usr/bin/env python3

# originally from https://github.com/Blutkoete/sighlp_cmp
# Modified to support signing github-generated source zip/tarball that is verified as unmodified.

import argparse
import hashlib
import os
import shutil
import sys
import tempfile
import urllib.request


EXITCODE_CONTENT_IS_EQUAL = 0
EXITCODE_CONTENT_IS_NOT_EQUAL_OR_ERROR = 1

VERBOSITY_LEVEL_SILENT = 0
VERBOSITY_LEVEL_NORMAL = 1
VERBOSITY_LEVEL_ULTRA = 2

verbosity_level = VERBOSITY_LEVEL_NORMAL

dir_names_to_ignore = []
file_names_to_ignore = []

def cond_print(what, condition):
    """
    Conditional print().
    :param what: What to print.
    :param condition: Condition to check.
    """
    if condition:
        print(what)


def parse_command_line_arguments():
    """
    Parses the command line arguments.
    :return: Parsed command line arguments.
    """
    parser = argparse.ArgumentParser('downloads a Github release archive and compares it to a local folder')
    parser.add_argument('-v', '--verbosity',
                        type=int,
                        choices=[VERBOSITY_LEVEL_SILENT, VERBOSITY_LEVEL_NORMAL, VERBOSITY_LEVEL_ULTRA],
                        dest='verbosity',
                        default=VERBOSITY_LEVEL_NORMAL,
                        help='set the verbosity level (0 - silent, 1 - normal, 2 - ultra)')
    parser.add_argument('-d', '--ignore-dir',
                        metavar='dir_name_to_ignore',
                        type=str,
                        action='append',
                        dest='dir_names_to_ignore',
                        help='directory name to ignore, e.g. ".git" - may be specified multiple times')
    parser.add_argument('-f', '--ignore-file',
                        metavar='file_name_to_ignore',
                        type=str,
                        action='append',
                        dest='file_names_to_ignore',
                        help='file name to ignore, e.g. ".gitignore" - may be specified multiple times')
    parser.add_argument('-s' '--save-download', metavar='save_download_dest', type=str, dest='save_download_dest') 
    parser.add_argument('url', metavar='url', type=str, help='download URL')
    parser.add_argument('path', metavar='path', type=str, help='path to local folder')
    return parser.parse_args()


def create_tmp_paths(url):
    """
    Create temporary paths for downloading/unarchiving.
    :param url: The URL of the file to download to extract a file name.
    :return: Triple of temporary dir, file path and temporary unarchiving dir path.
    """
    tmp_dir = tempfile.mkdtemp(prefix='sighlp_cmp_')
    # Take only the last part of the url as the file name - there are probably situations in which this simple approach
    # does not work
    tmp_file = os.path.join(tmp_dir, url.rsplit('/', 1)[-1])
    tmp_unarchived_dir = None
    return tmp_dir, tmp_file, tmp_unarchived_dir


def download_archive(url, dst):
    """
    Download the specified archive from the given url.
    :param url: Url to download.
    :param dst: Destination for the downloaded file.
    """
    global verbosity_level
    cond_print('Downloading "{}" ...'.format(url), verbosity_level > VERBOSITY_LEVEL_SILENT)
    urllib.request.urlretrieve(url, dst)
    cond_print('Download complete!', verbosity_level > VERBOSITY_LEVEL_SILENT)


def unpack_archive(src, dst):
    """
    Unpack given archive. The destination folder must contain the source file, but only the source file.
    :param src: Source file.
    :param dst: Destination folder.
    :return: Folder path unpacked from the archive.
    """
    global verbosity_level
    cond_print('Unpacking ...', verbosity_level > VERBOSITY_LEVEL_SILENT)
    shutil.unpack_archive(src, dst)
    cond_print('Unpacking complete!', verbosity_level > VERBOSITY_LEVEL_SILENT)
    tmp_dir_list = os.listdir(dst)
    if len(tmp_dir_list) != 2:
        raise RuntimeError('There are more than two entries in tmp directory after unpacking. Aborting.')
    if os.path.basename(src) not in tmp_dir_list:
        raise RuntimeError('Archive file not longer in tmp directory. Aborting.')
    for path in tmp_dir_list:
        if path == os.path.basename(src):
            continue
        else:
            return path


def get_sha512_hashdigest(file_path):
    """
    Returns the SHA512 hex digest for the given file.
    :param file_path: Path to the file.
    :return: SHA512 hex digest.
    """
    file_hash = hashlib.sha512()
    with open(file_path, 'rb') as f:
        while True:
            data = f.read(1024)
            if not data:
                break
            file_hash.update(data)
    return file_hash.hexdigest()


def compare_folders(folder1, folder2):
    """
    Compares two folders and all files and subfolders in them.
    :param folder1: Path to folder 1.
    :param folder2: Path to folder 2.
    :return: True on success, else an exception is raised.
    """
    global verbosity_level, dir_names_to_ignore, file_names_to_ignore
    cond_print('Comparing "{}" to "{}" ...'.format(folder1, folder2), verbosity_level > VERBOSITY_LEVEL_SILENT)
    folder1_dir_list = []
    folder1_file_list = []
    folder2_dir_list = []
    folder2_file_list = []
    # Get all directories & files as lists for folder1
    for root, dirs, files in os.walk(folder1):
        for ignored_dir in dir_names_to_ignore:
            if ignored_dir in dirs:
                dirs.remove(ignored_dir)
                cond_print('Ignoring directory "{}".'.format(os.path.join(root, ignored_dir)), verbosity_level > VERBOSITY_LEVEL_SILENT)
        for dir_ in dirs:
            full_dir_path = os.path.join(root, dir_)
            clean_dir_path = full_dir_path.replace(folder1, '', 1)
            folder1_dir_list.append(clean_dir_path)
        for file_ in files:
            full_file_path = os.path.join(root, file_)
            if file_ in file_names_to_ignore:
                cond_print('Ignoring file "{}".'.format(full_file_path), verbosity_level > VERBOSITY_LEVEL_SILENT)
                continue
            clean_file_path = full_file_path.replace(folder1, '', 1)
            folder1_file_list.append(clean_file_path)
    # Get all directories & files as lists for the local path
    for root, dirs, files in os.walk(folder2):
        for ignored_dir in dir_names_to_ignore:
            if ignored_dir in dirs:
                dirs.remove(ignored_dir)
                cond_print('Ignoring directory "{}".'.format(os.path.join(root, ignored_dir)), verbosity_level > VERBOSITY_LEVEL_SILENT)
        for dir_ in dirs:
            full_dir_path = os.path.join(root, dir_)
            clean_dir_path = full_dir_path.replace(folder2, '', 1)
            folder2_dir_list.append(clean_dir_path)
        for file_ in files:
            full_file_path = os.path.join(root, file_)
            if file_ in file_names_to_ignore:
                cond_print('Ignoring file "{}".'.format(full_file_path), verbosity_level > VERBOSITY_LEVEL_SILENT)
                continue
            clean_file_path = full_file_path.replace(folder2, '', 1)
            folder2_file_list.append(clean_file_path)
    folder1_dir_list.sort()
    folder1_file_list.sort()
    folder2_dir_list.sort()
    folder2_file_list.sort()
    # Now we have four lists: One directory list per root folder, one file list per root folder, all cleaned from their
    # specific prefix and sorted alphabetically. The lists must have the same length and contain the same entries in
    # the same order, else the root folders are not the same from our logic's point of view. For files, also compare
    # the hash.
    if len(folder1_dir_list) != len(folder2_dir_list):
        for folder1_dir in folder1_dir_list:
            if folder1_dir not in folder2_dir_list:
                cond_print('Folder 1 directory "{}" has no match in folder 2.'.format(folder1_dir), verbosity_level > VERBOSITY_LEVEL_SILENT)
        for folder2_dir in folder2_dir_list:
            if folder2_dir not in folder1_dir_list:
                cond_print('Folder 2 directory "{}" has no match in folder 1.'.format(folder2_dir), verbosity_level > VERBOSITY_LEVEL_SILENT)
        raise RuntimeError('Directory structure is not equal. Aborting.')
    for folder1_dir, folder2_dir in zip(folder1_dir_list, folder2_dir_list):
        if folder1_dir != folder2_dir:
            raise RuntimeError('Directory name not equal: "{}" in folder 1, "{}" on folder 2. Aborting'.format(folder1_dir, folder2_dir))
        cond_print('Comparison passed: "{}" and "{}".'.format(folder1_dir, folder2_dir), verbosity_level > VERBOSITY_LEVEL_NORMAL)
    # Compare files via name & hash
    if len(folder1_file_list) != len(folder2_file_list):
        for folder1_file in folder1_file_list:
            if folder1_file not in folder2_file_list:
                print('folder1 file "{}" has no match on folder2.'.format(folder1_file))
        for folder2_file in folder2_file_list:
            if folder2_file not in folder1_file_list:
                print('folder2 file "{}" has no match in folder1.'.format(folder2_file))
        raise RuntimeError('File count different. Aborting.')
    for folder1_file, folder2_file in zip(folder1_file_list, folder2_file_list):
        if folder1_file != folder2_file:
            raise RuntimeError('File name not equal: "{}" in folder1, "{}" on folder2. Aborting'.format(folder1_file, folder2_file))
        folder1_file_hash = get_sha512_hashdigest(os.path.join(folder1, folder1_file))
        folder2_file_hash = get_sha512_hashdigest(os.path.join(folder2, folder2_file))
        if folder1_file_hash != folder2_file_hash:
            raise RuntimeError('Hash mismatch: "{}" for folder 1 file "{}", "{}" for folder 2 file "{}".'.format(folder1_file_hash, folder1_file, folder2_file_hash, folder2_file))
        cond_print('Comparison passed: "{}" and "{}" [{}].'.format(folder1_file, folder2_file, folder1_file_hash), verbosity_level > VERBOSITY_LEVEL_NORMAL)
    return True


def clean_up_tmp_dir(tmp_dir):
    """
    Cleans the temporary used directories and files.
    :param tmp_dir: The tmp dir to clean.
    :param tmp_file: The tmp downloaded file.
    :param tmp_unarchived_dir: The tmp folder that contains the unarchived data.
    """
    for root, dirs, files in os.walk(tmp_dir, topdown=False):
        for file_ in files:
            os.remove(os.path.join(root, file_))
        for dir_ in dirs:
            os.rmdir(os.path.join(root, dir_))
    os.rmdir(tmp_dir)


def sighlp_cmp():
    """
    Main functionality.
    :return: 0 if compared equal, 1 else or on error.
    """
    global verbosity_level, dir_names_to_ignore, file_names_to_ignore
    # Preset result to False.
    content_is_equal = False
    # Parse commandline arguments
    args = parse_command_line_arguments()
    verbosity_level = args.verbosity
    if args.dir_names_to_ignore is not None:
        dir_names_to_ignore = args.dir_names_to_ignore
    if args.file_names_to_ignore is not None:
        file_names_to_ignore = args.file_names_to_ignore
    tmp_dir, tmp_file, tmp_unarchived_dir = create_tmp_paths(args.url)
    # This is the main logic: Try to download the file, extract it, convert the file and directory lists to a sorted,
    # easily comparable format and then compare everyhting (directories by name, files by name and hash).
    try:
        # Download the given file
        download_archive(args.url, tmp_file)
        # Save a copy if so specified
        if args.save_download_dest is not None:
            shutil.copyfile(tmp_file, args.save_download_dest)
        # Unpack it
        tmp_unarchived_dir = unpack_archive(tmp_file, tmp_dir)
        local_root_dir = args.path
        # Clean the paths
        if not tmp_unarchived_dir.endswith('/'):
            tmp_unarchived_dir = tmp_unarchived_dir + '/'
        if not local_root_dir.endswith('/'):
            local_root_dir = local_root_dir + '/'
        content_is_equal = compare_folders(os.path.join(tmp_dir, tmp_unarchived_dir), local_root_dir)
    except Exception as err:
        print(err, file=sys.stderr)
    finally:
        clean_up_tmp_dir(tmp_dir)
    if content_is_equal:
        cond_print('Comparison passed!', verbosity_level > VERBOSITY_LEVEL_SILENT)
        return EXITCODE_CONTENT_IS_EQUAL
    else:
        cond_print('Comparision failed.', verbosity_level > VERBOSITY_LEVEL_SILENT)
        return EXITCODE_CONTENT_IS_NOT_EQUAL_OR_ERROR


if __name__ == '__main__':
    sys.exit(sighlp_cmp())
