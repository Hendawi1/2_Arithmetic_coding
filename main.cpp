#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <random>
#include <string>


using namespace std;


struct characteristics { 
	size_t freqCount;
	size_t cumulativefreq;
	characteristics(size_t freq = 0) {
		freqCount = freq;
		cumulativefreq = 0;
	}
	characteristics(size_t freq, size_t cumulfreq) {
		freqCount = freq;
		cumulativefreq = cumulfreq;
	}
};



vector<unsigned char> VecToStr(vector<unsigned char> vec, size_t bits) {

	size_t bytes = ((bits - 1) / 8) + 1; // Количество байт
	vector<unsigned char> str;
	for (size_t i = 0; i < bits + 1; i++) str.push_back(0); 
	size_t strIndex = 0;
	for (size_t i = 0; i < bytes; i++) {
		unsigned char mask = 1 << 7; // Создаем маску, начиная с самого левого бита
		for (int j = 0; j < 8 && i * 8 + j < bits; j++) {
			if ((vec[i] & mask) != 0)
				str[strIndex] = '1';
			else
				str[strIndex] = '0';
			mask = mask >> 1;
			strIndex++;
		}
		mask = 1 << 7;
	}
	str[bits] = '\0';
	return str;
}


vector<unsigned char> encode(string &text, size_t &size, size_t &num_of_char, map<char, characteristics> &statistics){
	vector<unsigned char> encodedText;
	vector<unsigned char> mask;


	for (size_t i = 0; i < 8; i++) mask.push_back(1 << (7 - i));
	// mask = {10000000, 01000000, 00100000, 00010000, 00001000, 00000100, 00000010, 00000001}
	size_t temp = 0 ,left = 0 ,bits_to_follow = 0 ,right = 65535 ,First_quarter = (right / 4) + 1 ,Second_quarter = First_quarter * 2 ,Third_quarter = First_quarter * 3;
	
	/*
	المتغيران left و right هما الحدود الدنيا والعليا للنطاق الذي سيتم تمثيل الرموز فيه. في البداية، يتم تعيين left إلى 0 و right إلى 65535 (القيمة القصوى لعدد صحيح غير موقع من 16 بت).
	
	المتغيرات First_quarter و Second_quarter و Third_quarter هي حدود الأرباع الثلاثة الأولى من النطاق [left, right]. تُحسب هذه القيم بتقسيم right على 4 وإضافة 1 إلى الناتج (لضمان أن First_quarter تكون أكبر من 0)، ثم ضرب First_quarter بـ 2 و 3 للحصول على Second_quarter و Third_quarter على التوالي.
	*/
	
	

	size = 0; // لتخزين حجم النص المشفر
	for (size_t i = 0; text[i]; i++){
		temp = left;
		left = left + (statistics[text[i]].cumulativefreq - statistics[text[i]].freqCount) * (right - left) / num_of_char;
		right = temp + statistics[text[i]].cumulativefreq * (right - temp) / num_of_char - 1;

	/*
	
	- يتم تخزين القيمة الحالية لـ left في temp مؤقتًا.
	- يتم حساب left الجديدة باستخدام المعادلة:
	  left = left + (statistics[text[i]].cumulativefreq - statistics[text[i]].freqCount) * (right - left) / num_of_sym;
	  هذه المعادلة تضمن أن تكون left الجديدة في النطاق الصحيح للرمز الحالي، بناءً على تكراره وتكرار الرموز السابقة.
	- يتم حساب right الجديدة باستخدام المعادلة:
	  right = temp + statistics[text[i]].cumulativefreq * (right - temp) / num_of_sym - 1;
	  هذه المعادلة تضمن أن تكون right الجديدة في النطاق الصحيح للرمز الحالي، بناءً على تكراره وتكرار الرموز السابقة.

	بعد تحديث left و right، يتم تنفيذ باقي الكود في الحلقة لإضافة البتات المناسبة إلى encodedText وفقًا لموقع left و right ضمن النطاق [First_quarter, Third_quarter].
	
	*/

		while (true) {
			if (right < Second_quarter){
				size++;
				if (size % 8 == 1) encodedText.push_back(0);
				for (; bits_to_follow > 0; bits_to_follow--) {
					size++;
					if (size % 8 == 1) encodedText.push_back(0);
					encodedText[(size - 1) / 8] = encodedText[(size - 1) / 8] | mask[(size - 1) % 8];
				}
			}
			else if (left >= Second_quarter) {
				size++;
				if (size % 8 == 1) encodedText.push_back(0);
				encodedText[(size - 1) / 8] = encodedText[(size - 1) / 8] | mask[(size - 1) % 8];
				for (; bits_to_follow > 0; bits_to_follow--) {
					size++;
					if (size % 8 == 1) encodedText.push_back(0);
				}
				right -= Second_quarter;
				left -= Second_quarter;
			}
			else if ((left >= First_quarter) && (right < Third_quarter)) {
				bits_to_follow++; // добавляем биты условно
				left -= First_quarter;
				right -= First_quarter;
			}
			else break;

			right += right + 1;
			left += left;
		}
	}


	// يتم إضافة البتات الأخيرة إلى encodedText لتمثيل الفترة النهائية الضيقة التي تمثل النص المضغوط بأكمله
	bits_to_follow += 1;
	if (left < First_quarter) {
		size++;
		if (size % 8 == 1) encodedText.push_back(0);
		for (; bits_to_follow > 0; bits_to_follow--) {
			size++;
			if (size % 8 == 1) encodedText.push_back(0);
			encodedText[(size - 1) / 8] = encodedText[(size - 1) / 8] | mask[(size - 1) % 8];
		}
	}
	else {
		size++;
		if (size % 8 == 1) encodedText.push_back(0);
		encodedText[(size - 1) / 8] = encodedText[(size - 1) / 8] | mask[(size - 1) % 8];
		for (; bits_to_follow > 0; bits_to_follow--) {
			size++;
			if (size % 8 == 1) encodedText.push_back(0);
		}
	}
	
	
	cout << "encodedText: ";
	vector<unsigned char> vec = VecToStr(encodedText, size);
	for (size_t i = 0; i < size; i++) cout << vec[i];
	cout << endl;
	return encodedText;
}



string decode(vector<unsigned char> &encodedText, size_t &encodedSize, size_t &num_of_char, map<char, characteristics> &statistics) {
	string decodedText = "";
	if (encodedSize == 0) {
		cout << "ERROR OF ENCODING! \n";
		return decodedText;
	}
	cout << "encodedText: ";

	vector<unsigned char> mask;
	for (size_t i = 0; i < 8; i++) mask.push_back(1 << (7 - i));
	size_t left = 0, right = 65535, temp = 0, First_quarter = (right / 4) + 1, Second_quarter = First_quarter * 2, Third_quarter = First_quarter * 3, freq = 0, j = 16, value = 0;
	value = encodedText[0] << 8; // Сдвигаем старший байт на 8 бит влево
	if (encodedSize > 8) value |= encodedText[1]; // Записываем младший байт
	for (size_t i = 0; i < num_of_char; i++) {
		freq = ((value + 1 - left) * num_of_char - 1) / (right - left + 1);
		char symbol = ' ';
		// ищем символ
		for (auto it : statistics) {
			if (it.second.cumulativefreq > freq) {
				symbol = it.first;
				break;
				/*
				
				إذا كان التكرار التراكمي (`cumulativefreq`) للرمز الحالي أكبر من `freq` المحسوبة، فهذا يعني أن هذا هو الرمز المطلوب.
				
				z*/
			}
		}
		decodedText += symbol; // нашли символ, добавили к раскодированному тексту



		/*
		
		نقوم بتحديث `left` و `right` بناءً على إحصائيات الرمز `symbol` الموجود، وتهيئ النطاق لفك ضغط الرمز التالي.
		обновляем «левый» и «правый» на основе freqCount существующего «символа» и инициализируем область действия для распаковки следующего символа.
		
		*/

		temp = left;
		left = left + (statistics[symbol].cumulativefreq - statistics[symbol].freqCount) * (right - left) / num_of_char;
		right = temp + statistics[symbol].cumulativefreq * (right - temp) / num_of_char - 1;
		
		
		while (true) {
			if (right < Second_quarter) { /*Пропускаем 0 в закодированной последовательности*/ }
			else if (left >= Second_quarter) {
				value -= Second_quarter;
				left -= Second_quarter;
				right -= Second_quarter;
			}
			else if ((left >= First_quarter) && (right < Third_quarter)) {
				value -= First_quarter;
				left -= First_quarter;
				right -= First_quarter;
			}
			else break;
			
			left += left;
			right += right + 1;
			value += value;
			if (j < encodedSize) {
				if ((encodedText[j / 8] | mask[j % 8]) == encodedText[j / 8]) value |= 1;
				j++;
			}
		}
	}
	return decodedText;
}



int main() {
	//encode:
	cout << "\n~~~~~~~~~[ENCODE]~~~~~~~~~\n";
	ifstream input("text.txt");
	if (!input.is_open()) {
		cout << "file 'text.txt' not found\n";
		return 1;
	}
	ofstream output("encodedtext.bin", ios::binary | ios::out);
	string text, line;
	while (getline(input, line)) text += line;
	if (text.length() == 0) {
		cout << "text.txt is empty\n";
		exit(1);
	}
	// map < key  : value                  >
	// map < symb : struct characteristics >
	map<char, characteristics> statistics;
	size_t size = 0, temp = 0, tempSize = 0, num_of_char = 0;

	// for every char in text
	for (const char c : text) {
		statistics[c].freqCount++;
		num_of_char++;
	}
	for (auto i : statistics) {
		temp = statistics[i.first].freqCount;
		tempSize += temp;
		statistics[i.first] = characteristics(temp, tempSize);
		cout << i.first << ", " << statistics[i.first].freqCount << ", " << statistics[i.first].cumulativefreq << endl;
	}
	vector<unsigned char> encodedText = encode(text, size, num_of_char, statistics);
	output << size << ' ' << num_of_char << "\n";
	cout << "size: " << size << endl;
	cout << "num_of_char: " << num_of_char;
	// записываем в файл закодированный текст вместе с таблицей частот
	for (size_t i = 0; i < ((size - 1) / 8 + 1); i++) output << encodedText[i];
	output << "\n\n";
	for (auto &pair : statistics) output << pair.first << pair.second.freqCount << ' ';


	input.close();
	output.close();
	size = num_of_char = 0;

	cout << "\n\n\n\n~~~~~~~~~[DECODE]~~~~~~~~~\n";


	// decode:
	ifstream input_("encodedtext.bin", ios::binary | ios::in);
	ofstream output_("text.txt");
	if (!input_.is_open()) {
		cout << "file 'encodedtext.txt' not found\n";
		return 1;
	}
	size_t sumfreq = 0, freq = 0;
	bool flag = false, backspace = false;
	char sym = ' ';
	getline(input_, line);
	// побитово считываем длину кода и количество символов в тексте 
	size_t k = 0;
	for (; line[k] != ' ' && line[k]; k++) size = (size*10) + (line[k] - 48);
	for (size_t n = k + 1; line[n]; n++) num_of_char = (num_of_char*10) + (line[n] - 48);
	cout << "size: " << size << endl;
	cout << "num_of_char: " << num_of_char << endl;

	while (getline(input_, line)) {
		if (line.length() == 0) break; // отступ между деревом и кодом
		if (flag) encodedText.push_back(10); // передали байт == \n
		for (size_t i = 0; i < line.length(); i++) encodedText.push_back(line[i]);
		flag = true;
	}
	getline(input_, line);
	for (size_t i = 0; line[i];) {
		sym = line[i];
		freq = 0;
		size_t j = i + 1;
		for (; line[j] != ' ' && line[j]; j++) freq = (freq*10) + (line[j] - 48);
		i = j + 1;
		sumfreq += freq;
		statistics[sym] = characteristics(freq, sumfreq);
		cout << "sym: " << sym << " freq: " << freq << endl;
	}
	string decodedText = decode(encodedText, size, num_of_char, statistics);
	output_ << decodedText;
	cout << decodedText;
	return 0;
}
