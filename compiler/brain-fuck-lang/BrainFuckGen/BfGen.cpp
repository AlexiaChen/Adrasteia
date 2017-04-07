#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>



/*string transform table*/
 std::string str_trans_table[256][256];

static void usage(){

	fprintf(stderr,"The Usage: \n"          
		"BfGen  [text_file] [bf_code_file]\n"                            
		);
	exit(1);

}

static inline void error_msg(char* msg){

	fprintf(stderr,msg);
	exit(1);
}


static std::string repeat(std::string &s, int n) {
	
	std::string b;
	
	for (int i = 0; i < n; i++) b.append(s);
	return b;
}

/* initial state for str_trans_table[x][y]: go from x to y using +s or -s.*/    
static void init_table(std::string str_table[][256]){

	for(int x = 0; x < 256; ++x){
		for(int y = 0; y < 256; ++y){

			int delta = y - x;

			if (delta > 128) delta -= 256;
			if (delta < -128) delta += 256;

			if (delta >= 0) {
				str_table[x][y] = repeat( std::string("+"), delta);
			} else {
				str_table[x][y] = repeat( std::string("-"), -delta);
			}
		}
	}//end for int x

	// keep applying rules until we can't find any more shortenings                           
	bool iter = true;
	while (iter) {
		iter = false;

		// multiplication by n/d                                                                
		for (int x = 0; x < 256; x++) {
			for (int n = 1; n < 40; n++) {
				for (int d = 1; d < 40; d++) {
					int j = x;
					int y = 0;
					for (int i = 0; i < 256; i++) {
						if (j == 0) break;
						j = (j - d + 256) & 255;
						y = (y + n) & 255;
					}
					if (j == 0) {
						std::string s = "[" + repeat(std::string("-"), d) + ">" + repeat(std::string("+"), n) + "<]>";
						if (s.length() < str_table[x][y].length()) {
							str_table[x][y] = s;
							iter = true;
						}
					}

					j = x;
					y = 0;
					for (int i = 0; i < 256; i++) {
						if (j == 0) break;
						j = (j + d) & 255;
						y = (y - n + 256) & 255;
					}
					if (j == 0) {
						std::string s = "[" + repeat(std::string("+"), d) + ">" + repeat(std::string("-"), n) + "<]>";
						if (s.length() < str_table[x][y].length()) {
							str_table[x][y] = s;
							iter = true;
						}
					}
				}
			}
		}

		// combine number schemes                                                               
		for (int x = 0; x < 256; x++) {
			for (int y = 0; y < 256; y++) {
				for (int z = 0; z < 256; z++) {
					if (str_table[x][z].length() + str_table[z][y].length() < str_table[x][y].length()) {
						str_table[x][y] = str_table[x][z] + str_table[z][y];
						iter = true;
					}
				}
			}
		}
	}

}

static long get_file_len(FILE *fp){
	
	fseek(fp, 0, SEEK_END);
	 
	return ftell(fp);

}


static void generate_code(std::string &str, FILE* fp){
	
	char last_ch = 0;
	std::string::iterator itera;
	
	init_table(str_trans_table);
	//foreach
	for(itera = str.begin(); itera != str.end(); ++itera){
		
		std::string a = str_trans_table[last_ch][*itera];
		std::string b = str_trans_table[0][*itera];


		if (a.length() <= b.length()) {
			fprintf(fp,a.c_str());
		} else {
			
			fprintf(fp,">");
			fprintf(fp,b.c_str());
		}
		
		fprintf(fp,".");
		last_ch = *itera;
	}
#ifdef WIN32
	fprintf(fp,"\r\n");
#else
	fprintf(fp,"\n");
#endif

}

static void gen_file(char* argv[]){

	FILE *fp_in  = fopen(argv[1],"r");
	FILE *fp_out = fopen(argv[2],"w");

	if(NULL == fp_in){
		error_msg("cannot open the text file\n");
	}
	
	if(NULL == fp_out){
		error_msg("cannot create the .bf file\n");
	}

	int file_len = get_file_len(fp_in);
	
	rewind(fp_in);
	
	char c;
	std::string s;
	
	while((c = fgetc(fp_in)) != EOF){
		
		int v = c;
		
		if (v < 0) break;
		if (v == 0) continue; // no zeros                                                       
		
		s.push_back((char)v);
		
		
	}

	generate_code(s,fp_out);

	fclose(fp_in);
	fclose(fp_out);


}


int main(int argc, char* argv[]){

	if(argc != 3){
		usage();
	}


	gen_file(argv);

	

	return 0;
}
