#include <spvm/types.h>

void spvm_string_read(spvm_source spv, spvm_string str, spvm_word length)
{
	while (length) {
		spvm_word word = SPVM_READ_WORD(spv);

		str[0] = (word & 0x000000FF);
		str[1] = (word & 0x0000FF00) >> 8;
		str[2] = (word & 0x00FF0000) >> 16;
		str[3] = (word & 0xFF000000) >> 24;
		str += 4;

		length--;
	}
}
spvm_string spvm_string_read_all(spvm_source spv, spvm_word* length)
{
	spvm_word word_count = 8;
	spvm_string ret = (spvm_string)malloc(word_count * sizeof(spvm_word));
	spvm_byte run = 1;

	spvm_string start = ret;

	while (run) {
		spvm_word word = SPVM_READ_WORD(spv);

		ret[0] = (word & 0x000000FF);
		ret[1] = (word & 0x0000FF00) >> 8;
		ret[2] = (word & 0x00FF0000) >> 16;
		ret[3] = (word & 0xFF000000) >> 24;
		(*length)++;

		if (ret[3] == 0)
			run = 0;
		else if (*length >= word_count) {
			word_count += 8;
			ret = start = (spvm_string)realloc(start, word_count * sizeof(spvm_word));
			ret += (*length) * 4;
		}
		else ret += 4;
	}

	return start;
}