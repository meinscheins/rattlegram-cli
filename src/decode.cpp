#include "decoder.hh"
#include "wav.hh"
#include <cstdint>
#include "AudioFile.h"

static DecoderInterface *decoder;

int main(int argc, char **argv) {
	if (argc < 3 || argc > 4) {
        std::cerr << "usage: " << argv[0] << " FILE CHANNEL [MAPPING]" << std::endl;
        return 1;
    }
	const char* input_name = argv[1];
	int channel = std::atoi(argv[2]);
	int psk = 4;
	if (argc > 3)
		psk = std::atoi(argv[3]);
	AudioFile<int16_t> audioFile;
	audioFile.load(input_name);
	int rate = audioFile.getSampleRate();
	int channel_count = audioFile.getNumChannels();
	if (channel < 0 || channel > 4) {
		std::cout << "Channel must be between 0 and 5." << std::endl;
		return 1; 
	}
	if (!(channel_count == 1 || channel_count == 2)) {
		std::cout << "Only mono and stereo audio is supported." << std::endl;
		return 1;
	}
	if ((channel > 0 && channel < 5) && (channel_count != 2)) {
		std::cout << "Channel 1 - 4 are only supported for stereo audio." << std::endl;
		return 1; 
	}
	if (channel_count == 1) {
	std::cout << "Mono audio. Channel set to 0." << std::endl;
	channel = 0;
	}
	int file_length = audioFile.getNumSamplesPerChannel() * channel_count;
	int symbol_length = (1280 * rate) / 8000;
	int guard_length = symbol_length / 8;
	int extended_length = symbol_length + guard_length;
	int record_count = rate/50;
	//int16_t file[file_length + 22 * record_count];
	int16_t* file = new int16_t[file_length + 22 * record_count];
	for (int i = 0; i < file_length/channel_count; i++) {
		for (int c = 0; c < channel_count; c++) {
			file[i * channel_count + c] = audioFile.samples[c][i];
		}
	}

    switch (psk) {
        case 2:
            decoder = new(std::nothrow) Decoder<8000, 2>;
            break;
        case 4:
            decoder = new(std::nothrow) Decoder<8000, 4>;
            break;
        case 8:
            decoder = new(std::nothrow) Decoder<8000, 8>;
            break;
		case 16:
            decoder = new(std::nothrow) Decoder<8000, 16>;
            break;
        default:
            std::cerr << "Unsupported symbol mapping." ;
            std::cerr << "Supported PSK: 2/4/8. Supported QAM: 16."<< std::endl;
            return 1;
    }
            
  
    

	for (int i = 0; i * record_count * channel_count < file_length + 10 * channel_count * record_count; i++) {
		if (decoder->feed(&file[i*record_count*channel_count], record_count, channel)) {
			int status = decoder->process();
			float cfo = -1.0;
			int32_t mode = -1;
			uint8_t call_sign[192] = {0}; 
			uint8_t payload[170] = {0};
			int timestamp = i * record_count * channel_count/8000;
 
			switch (status) {
				case 0:
					break;
				case 1:
					std::cout << "PREAMBLE FAIL" << std::endl;
					switch (psk) {
    				    case 2:
    				        decoder = new(std::nothrow) Decoder<8000, 2>;
    				        break;
    				    case 4:
    				        decoder = new(std::nothrow) Decoder<8000, 4>;
    				        break;
    				    case 8:
    				        decoder = new(std::nothrow) Decoder<8000, 8>;
    				        break;
						case 16:
    				        decoder = new(std::nothrow) Decoder<8000, 16>;
    				        break;
    				    default:
    				        std::cerr << "Unsupported symbol mapping." ;
    				        std::cerr << "Supported PSK: 2/4/8. Supported QAM: 16."<< std::endl;
    				        return 1;
    				}
					break;
				case 2:
					decoder->staged(&cfo, &mode, call_sign);
					std::cout << "SYNC:" << std::endl 
							<< "CFO: " 
							<< cfo << std::endl 
							<< "Mode: "
							<< mode << std::endl 
							<< "Call sign: "
							<< (reinterpret_cast<char*>(call_sign))
							<< std::endl;
					break;
				case 3:
					std::cout << "Time: " << timestamp << " Bit flips: " << decoder->fetch(payload) << std::endl;
					std::cout << "DONE:  payload: " 
							<< (reinterpret_cast<char*>(payload))
							<< std::endl;
					switch (psk) {
    				    case 2:
    				        decoder = new(std::nothrow) Decoder<8000, 2>;
    				        break;
    				    case 4:
    				        decoder = new(std::nothrow) Decoder<8000, 4>;
    				        break;
    				    case 8:
    				        decoder = new(std::nothrow) Decoder<8000, 8>;
    				        break;
						case 16:
    				        decoder = new(std::nothrow) Decoder<8000, 16>;
    				        break;
    				    default:
    				        std::cerr << "Unsupported symbol mapping." ;
    				        std::cerr << "Supported PSK: 2/4/8. Supported QAM: 16."<< std::endl;
    				        return 1;
    				}
					break;
				case 4:
					std::cout << "HEAP ERROR" << std::endl;
					switch (psk) {
    				    case 2:
    				        decoder = new(std::nothrow) Decoder<8000, 2>;
    				        break;
    				    case 4:
    				        decoder = new(std::nothrow) Decoder<8000, 4>;
    				        break;
    				    case 8:
    				        decoder = new(std::nothrow) Decoder<8000, 8>;
    				        break;
						case 16:
    				        decoder = new(std::nothrow) Decoder<8000, 16>;
    				        break;
    				    default:
    				        std::cerr << "Unsupported symbol mapping." ;
    				        std::cerr << "Supported PSK: 2/4/8. Supported QAM: 16."<< std::endl;
    				        return 1;
    				}
					break;
				case 5:
					decoder->staged(&cfo, &mode, call_sign);
					std::cout << "NOPE:"
							<< "  CFO "
							<< cfo
							<< "  mode: "
							<< mode
							<< "  call sign: "
							<< (reinterpret_cast<char*>(call_sign))
							<< std::endl;
					break;
				case 6: 
					decoder->staged(&cfo, &mode, call_sign);
					std::cout << "PING:"
							<< "  CFO "
							<< cfo
							<< "  mode: "
							<< mode
							<< "  call sign: "
							<< (reinterpret_cast<char*>(call_sign))
							<< std::endl;
					break;
				default:
					std::cout << "UNDEFINED BEHAVIOUR" << std::endl;
					return 1;

			}
		}
	}
	delete[] file;
    return 0;
}