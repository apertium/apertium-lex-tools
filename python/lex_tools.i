%module lextools

%include <lrx_processor.h>
%include <lttoolbox/lt_locale.h>

%typemap(in) char ** {
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *py_obj = PyList_GetItem($input, i);
      if (PyUnicode_Check(py_obj)) {
        $1[i] = strdup(PyUnicode_AsUTF8(py_obj));
      }
      else {
        PyErr_SetString(PyExc_TypeError, "list must contain strings");
        free($1);
        return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
}

%typemap(freearg) char ** {
  free((char *) $1);
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

  void lrx_proc(char argc, char **argv, char *input_path, char *output_path)
  {
    bool useMaxEnt = false;
    FILE* input = fopen(input_path, "r");
    FILE* output = fopen(output_path, "w");
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
          useMaxEnt = true;
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
