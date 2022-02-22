#include <filesystem>
#include <fstream>

int main(int argc, char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		std::filesystem::path filename(argv[i]);
		std::ifstream input(filename, std::ios::binary);
		filename.replace_filename(filename.stem().wstring() + L"-converted" + filename.extension().wstring());
		std::ofstream output(filename, std::ios::binary);

		output << (unsigned char)0x20;
		for (int i = 0; i < 256; ++i) {
			unsigned int color;
			input.read((char*)&color, 4);
			if (i > 0) color |= 0xff000000;
			output.write((char*)&color, 4);
		}
	}
	return 0;
}
