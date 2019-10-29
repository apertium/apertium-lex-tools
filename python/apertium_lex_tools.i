%module apertium_lex_tools

%include <lrx_processor.h>
%include <lttoolbox/lt_locale.h>

%typemap(in) (int argc, char **argv) {
  if (PyTuple_Check($input)) {
    int i = 0;
    $1 = PyTuple_Size($input);
    $2 = (char **) malloc(($1 + 1)*sizeof(char *));
    for (i = 0; i < $1; i++) {
      PyObject *py_obj = PyTuple_GetItem($input, i);
      if (PyUnicode_Check(py_obj)) {
        $2[i] = strdup(PyUnicode_AsUTF8(py_obj));
      }
      else {
        PyErr_SetString(PyExc_TypeError, "tuple must contain strings");
        free($2);
        return NULL;
      }
    }
    $2[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not a tuple");
    return NULL;
  }
}

%typemap(freearg) (int argc, char **argv) {
  free((char *) $2);
}

%inline%{
#define SWIG_FILE_WITH_INIT
#include <lrx_processor.h>

#include <getopt.h>

class LRXProc: public LRXProcessor
{
public:
  /**
   * Imitates functionality of lrx_proc using file path
   */
  LRXProc(char *dictionary_path)
  {
    FILE *dictionary = fopen(dictionary_path, "rb");
    load(dictionary);
    fclose(dictionary);
  }

  void lrx_proc(int argc, char **argv, char *input_path, char *output_path)
  {
    FILE* input = fopen(input_path, "rb");
    FILE* output = fopen(output_path, "wb");
    optind = 1;
    while(true)
    {
      int c = getopt(argc, argv, "mztd");
      if(c == -1)
      {
        break;
      }

      switch(c)
      {
        case 'm':
          break;

        case 'z':
          setNullFlush(true);
          break;

        case 't':
          setTraceMode(true);
          break;

        case 'd':
          setDebugMode(true);
          break;
        default:
          break;
      }
    }
    processME(input, output);
    fclose(input);
    fclose(output);
  }
};

%}
