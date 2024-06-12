import os

def count_lines(directory):
    total_lines = 0

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.bat', '.py', '.cpp', '.h')):
                file_path = os.path.join(root, file)
                with open(file_path, 'r') as f:
                    lines = f.readlines()
                    total_lines += len(lines)
                    # print("- {} --> {} lines".format(file, len(lines)))

    return total_lines

# print("Counting lines...")

directory = os.path.dirname(os.path.realpath(__file__))
line_count = count_lines(directory)

print(f"Total lines: {line_count}")

def count_characters(directory):
    total_characters = 0

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.bat', '.py', '.cpp', '.h')):
                file_path = os.path.join(root, file)
                with open(file_path, 'r') as f:
                    characters = f.read()
                    total_characters += len(characters)
                    # print("- {} --> {} characters".format(file, len(characters)))

    return total_characters


def average_word_length(directory):
    total_words = 0
    total_characters = 0

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.bat', '.py', '.cpp', '.h')):
                file_path = os.path.join(root, file)
                with open(file_path, 'r') as f:
                    words = f.read().split()
                    total_words += len(words)
                    total_characters += sum(len(word) for word in words)

    return total_characters / total_words

def time_to_write(directory):
    awl = average_word_length(directory)
    
    time = (char_count / awl) / TYPING_SPEED
    
    # format: hh:mm:ss
    return "{:02d}:{:02d}:{:02d}".format(int(time // 60), int(time % 60), int((time % 1) * 60))

# print("\nCounting characters...")

char_count = count_characters(directory)

print(f"Total characters: {char_count}")
print("Total words: ", char_count / average_word_length(directory))

TYPING_SPEED = 65 # wpm

print("Time to write all the code: ", time_to_write(directory))

def size_in_mb(directory):
    total_bytes = 0

    for root, dirs, files in os.walk(directory):
        for file in files:
            # toate fisierele nu doar cele scrise
            file_path = os.path.join(root, file)
            total_bytes += os.path.getsize(file_path)

    return total_bytes / 1024 / 1024

# print("\nCalculating size...")
print(f"Total size: {size_in_mb(directory)} MB")

averageCharactersPerLine = char_count / line_count
print(f"Average characters per line: {averageCharactersPerLine}")

averageWordsPerLine = (char_count / average_word_length(directory)) / line_count
print(f"Average words per line: {averageWordsPerLine}")

averageLinesPerFile = line_count / len([name for name in os.listdir(directory) if os.path.isfile(name)])
print(f"Average lines per file: {averageLinesPerFile}")

averageCharactersPerFile = char_count / len([name for name in os.listdir(directory) if os.path.isfile(name)])
print(f"Average characters per file: {averageCharactersPerFile}")

averageWordsPerFile = (char_count / average_word_length(directory)) / len([name for name in os.listdir(directory) if os.path.isfile(name)])
print(f"Average words per file: {averageWordsPerFile}")

averageSizePerFile = size_in_mb(directory) / len([name for name in os.listdir(directory) if os.path.isfile(name)])
print(f"Average size per file: {averageSizePerFile} MB")

total_time = time_to_write(directory)
total_time = total_time.split(':')
total_time = int(total_time[0]) * 60 + int(total_time[1]) + int(total_time[2]) / 60
testing = total_time * 3
compiling = total_time * 0.2
running = total_time * 0.2
designing = total_time * 0.2
debugging = total_time * 0.4
thinking = total_time * 0.2
editing = total_time * 2
meta = total_time * 0.3
total_time = testing + compiling + running + designing + debugging + thinking + editing + meta
# convert back to hh:mm:ss
total_time = "{:02d}:{:02d}:{:02d}".format(int(total_time // 60), int(total_time % 60), int((total_time % 1) * 60))
print("Total time: ", total_time)