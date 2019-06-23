%module lextools

%{
#define SWIG_FILE_WITH_INIT
#include <lrx_processor.h>


class LRX: public LRXProcessor
{
public:
  /**
   * Imitates functionality of lrx_proc using file path
   */
  void lrx_proc(char arg, char *dictionary_path, char *input_path, char *output_path);
};


void
LRX::lrx_proc(char arg, char *dictionary_path, char *input_path, char *output_path)
{
  bool useMaxEnt = false;
  FILE *in = fopen(dictionary_path, "rb");
  load(in);
  FILE *input = fopen(input_path, "r"), *output = fopen(output_path, "w");
  switch(arg)
  {
    case 'm':
      useMaxEnt = true;
      break;
    default:
      useMaxEnt = false;
  }
  init();
  if(useMaxEnt)
  {
    processME(input, output);
  }
  else
  {
    process(input, output);
  }
  fclose(in);
  fclose(input);
  fclose(output);
}

%}


%include <lrx_processor.h>


class LRX: public LRXProcessor
{
public:
  /**
   * Imitates functionality of lrx_proc using file path
   */
  void lrx_proc(char arg, char *dictionary_path, char *input_path, char *output_path);
};
