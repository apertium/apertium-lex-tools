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
  LRX(char *dictionary_path)
  {
    FILE *dictionary = fopen(dictionary_path, "rb");
    load(dictionary);
    fclose(dictionary);
  }

  void lrx_proc(char arg, char *input_path, char *output_path)
  {
    bool useMaxEnt = false;
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
    fclose(input);
    fclose(output);
  }
};

%}


%include <lrx_processor.h>
%include <lttoolbox/lt_locale.h>


class LRX: public LRXProcessor
{
public:
  LRX(char *dictionary_path);
  void lrx_proc(char arg, char *input_path, char *output_path);
};
