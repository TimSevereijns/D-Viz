import argparse
import os
import sys
import shutil
import zipfile

def getArguments():
    parser = argparse.ArgumentParser(
        description='Script to deflate a ZIP archive.')

    parser.add_argument(
        '-i', '--input', type=str, help='Input directory path', required=True)
    parser.add_argument(
        '-o', '--output', type=str, help='Output directory path', required=True)

    args = parser.parse_args()
    return args.input, args.output

def unzip(inputDir, outputDir):
    if (os.path.isdir(outputDir)):
        shutil.rmtree(outputDir)

    os.mkdir(outputDir)

    with zipfile.ZipFile(inputDir, 'r') as zip_ref:
        zip_ref.extractall(outputDir)

def main():
    inputDir, outputDir = getArguments()

    print("Extracting \"" + inputDir + "\" to \"" + outputDir + "\"", flush=True)
    unzip(inputDir, outputDir)
    print("Done.")

if __name__ == "__main__":
    sys.exit(main())