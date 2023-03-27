// To compile for debugging: mark file dirty by adding and removing a char, then set `debugMode` to true in setup.py of pylcs, then run `pip install --global-option build --global-option --debug pylcs/` and it will install the debug build. (Based on https://stackoverflow.com/questions/30153936/how-to-compile-python-extension-with-debug-info-using-pip )

#ifdef LCS_DEBUG
#define PYBIND11_DEBUG // (used for include of pybind11 below)
#define DEBUG_PRINT(...) fprintf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <iostream>
#include <string.h>
#include <sstream>

#include "longestCommonSubsequence.hpp"
//extern "C" {
#include <mba/diff.h> // Based on https://www.ioplex.com/~miallen/libmba/dl/libmba-0.8.10.tar.gz , https://github.com/innerout/libmba with some patches for version 0.8.10 to work with CMake, since the newer one on GitHub doesn't seem to support Windows as easily (it might actually)
#include <mba/msgno.h>
//}

using namespace std; // https://stackoverflow.com/questions/31779419/allocator-is-ambigous-compile-error

namespace py = pybind11;


// https://github.com/pybind/pybind11/blob/master/tests/test_exceptions.cpp
// A type that should be raised as an exception in Python
class PylcsException : public std::exception {
public:
    explicit PylcsException(const char *m) : message{m} {}
    const char *what() const noexcept override { return message.c_str(); }

private:
    std::string message = "";
};


vector<string> utf8_split(const string &str){
    vector<string> split;
    int len = str.length();
    int left = 0;
    int right = 1;

    for (int i = 0; i < len; i++){
        if (right >= len || ((str[right] & 0xc0) != 0x80)){
            string s = str.substr(left, right - left);
            split.push_back(s);
            // printf("%s %d %d\n", s.c_str(), left, right);
            left = right;
        }
        right ++;
    }
    return split;
}


// 最长公共子序列（不连续）
int lcs_length_(const string &str1, const string &str2) {
    if (str1 == "" || str2 == "")
        return 0;
    vector<string> s1 = utf8_split(str1);
    vector<string> s2 = utf8_split(str2);
    int m = s1.size();
    int n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    int i, j;
    // printf("%d %d\n", m, n);

    for (i = 0; i <= m; i++) {
        dp[i][0] = 0;
    }
    for (j = 0; j <= n; j++) {
        dp[0][j] = 0;
    }
    for (i = 1; i <= m; i++) {
        for (j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                if (dp[i - 1][j] >= dp[i][j - 1])
                    dp[i][j] = dp[i - 1][j];
                else
                    dp[i][j] = dp[i][j-1];
            }
        }
    }
    return dp[m][n];
}

vector<vector<int>> lcs_matrix_(const string &str1, const string &str2) {
    if (str1 == "" || str2 == "")
        return vector<vector<int>>();
    vector<string> s1 = utf8_split(str1);
    vector<string> s2 = utf8_split(str2);
    int m = s1.size();
    int n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    int i, j;
    // printf("%d %d\n", m, n);

    for (i = 0; i <= m; i++) {
        dp[i][0] = 0;
    }
    for (j = 0; j <= n; j++) {
        dp[0][j] = 0;
    }
    for (i = 1; i <= m; i++) {
        for (j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                if (dp[i - 1][j] >= dp[i][j - 1])
                    dp[i][j] = dp[i - 1][j];
                else
                    dp[i][j] = dp[i][j-1];
            }
        }
    }
    return dp;
}

std::pair<string, std::vector<bool> /*true if a corresponding index in `str1` is in the LCS*/> lcs_string(const string &str1, const string &str2) {
  string retval;
  lcs3::members xs_in_lcs;
  lcs3::lcs(str1, str2, retval, xs_in_lcs);
  return {retval, xs_in_lcs};
}

class VArrayWrapper {
public:
  VArrayWrapper() :
    m_varray(nullptr),
    m_length(0)
  {}

  VArrayWrapper(varray* varray_, size_t length) :
    m_varray(varray_),
    m_length(length)
  {}

  VArrayWrapper(const VArrayWrapper&& other) :
    m_varray(other.m_varray),
    m_length(other.m_length)
  {
    // other.m_varray = nullptr;
    // other.m_length = 0;
  }

  VArrayWrapper(const VArrayWrapper& other) = delete;

  ~VArrayWrapper() {
    // FIXME: Memory leak currently. Uncomment the below and change; this crashes for some reason (double-free?)
    // if (varray_del(m_varray)) { // (Note: varray_del() supports nullptr arguments)
    //   // Error occurred:
    //   MNO(errno);
    //   exit(1);

    //   // can't get this to work:
    //   // PMNO(errno);
    //   // throw PylcsException(_msgno_buf);
    // }
  }
  
  size_t size() const {
    return m_length;
  }

  varray* getVArray() {
    return m_varray;
  }

protected:
  varray* m_varray;
  size_t m_length;
};

template <typename T>
class VArrayIterator {
public:
  VArrayIterator(VArrayWrapper& varrayWrapper) :
    m_varray(varrayWrapper.getVArray()),
    m_index(0),
    m_length(varrayWrapper.size())
  {
    //varray_iterate(m_varray, &m_iter); // (Note: varray_iterate() supports nullptr arguments)
  }

  T next() {
    if (m_index >= m_length) {
      DEBUG_PRINT(stderr, "m_index >= m_length\n");
      throw py::stop_iteration();
      //return nullptr;
    }

    //return *(T*)varray_next(m_varray, &m_iter);
    DEBUG_PRINT(stderr, "m_index: %p out of length %p\n", m_index, m_length);
    T retval = *(T*)varray_get(m_varray, m_index++);
    DEBUG_PRINT(stderr, "ptr: %p\n", retval);
    return retval;
  }

protected:
  unsigned int m_index;
  size_t m_length;
  varray* m_varray;
  //iter_t m_iter;
};

VArrayWrapper diff(const string &str1, const string &str2) {
  // Based on https://github.com/innerout/libmba/blob/master/examples/diff/strdiff.c
  int n, m, d;
  int sn, i;
  varray *ses = varray_new(sizeof(diff_edit), NULL);
  
  n = str1.size();
  m = str2.size();

  // printf("Got this far\n");
  // return {};
  if ((d = diff(str1.c_str(), 0, n, str2.c_str(), 0, m, NULL, NULL, NULL, 0, ses, &sn, NULL)) == -1) {
    // Error occurred:
    DEBUG_PRINT(stderr, "Error from diff() call\n");
    MNO(errno); //MMNO(errno);
    //return EXIT_FAILURE;
    //throw errno;
    return {};
  }

  // // Iteration demo:
  // const char* a = str1.c_str();
  // const char* b = str2.c_str();
  // for (size_t i = 0; i < sn; i++) {
  //   struct diff_edit *e = (diff_edit*)varray_get(ses, i);

  //   switch (e->op) {
  //   case DIFF_MATCH:
  //     printf("MAT: ");
  //     fwrite(a + e->off, 1, e->len, stdout);
  //     break;
  //   case DIFF_INSERT:
  //     printf("INS: ");
  //     fwrite(b + e->off, 1, e->len, stdout);
  //     break;
  //   case DIFF_DELETE:
  //     printf("DEL: ");
  //     fwrite(a + e->off, 1, e->len, stdout);
  //     break;
  //   }
  //   printf("\n");
  // }

  // Source: https://poe.com/
  // Create a NumPy array that wraps the varray data
  // py::array_t<diff_edit> my_array({sn}, {sizeof(diff_edit)}, ses);

  return {ses, (size_t)sn};
}


// 最长公共子串（连续）
int lcs2_length_(const string &str1, const string &str2) {
    if (str1 == "" || str2 == "")
        return 0;
    vector<string> s1 = utf8_split(str1);
    vector<string> s2 = utf8_split(str2);
    int m = s1.size();
    int n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    int i, j;
    int max = 0;

    for (i = 0; i <= m; i++) {
        dp[i][0] = 0;
    }
    for (j = 0; j <= n; j++) {
        dp[0][j] = 0;
    }
    for (i = 1; i <= m; i++) {
        for (j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > max){
                    max = dp[i][j];
                }
            }
            else {
                dp[i][j] = 0;
            }
        }
    }
    return max;
}


// TODO 返回子序列
int lcs(const string &str1, const string &str2){
    return lcs_length_(str1, str2);
}

vector<vector<int>> lcs_matrix(const string &str1, const string &str2){
    return lcs_matrix_(str1, str2);
}


// TODO 返回子串
int lcs2(const string &str1, const string &str2){
    return lcs2_length_(str1, str2);
}


vector<int> lcs_of_list(const string &str1, vector<string> &str_list){
    int size = str_list.size();
    vector<int> ls(size);
    for (int i = 0; i < size; i++){
        int l = lcs(str1, str_list[i]);
        ls[i] = l;
    }
    return ls;
}


vector<int> lcs2_of_list(const string &str1, vector<string> &str_list){
    int size = str_list.size();
    vector<int> ls(size);
    for (int i = 0; i < size; i++){
        int l = lcs2(str1, str_list[i]);
        ls[i] = l;
    }
    return ls;
}


// 编辑距离
int levenshtein_distance(const string &str1, const string &str2) {
    if (str1 == "" || str2 == "")
        return 0;
    vector<string> s1 = utf8_split(str1);
    vector<string> s2 = utf8_split(str2);
    int m = s1.size();
    int n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    int i, j;

    for (i = 0; i <= m; i++) {
        dp[i][0] = i;
    }
    for (j = 0; j <= n; j++) {
        dp[0][j] = j;
    }
    for (i = 1; i <= m; i++) {
        for (j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + min(min(dp[i][j - 1], dp[i - 1][j]), dp[i - 1][j - 1]);
            }
        }
    }
    return dp[m][n];
}


vector<int> levenshtein_distance_of_list(const string &str1, vector<string> &str_list){
    int size = str_list.size();
    vector<int> ls(size);
    for (int i = 0; i < size; i++){
        int l = levenshtein_distance(str1, str_list[i]);
        ls[i] = l;
    }
    return ls;
}


PYBIND11_MODULE(pylcs, m) {
    // https://dmtn-014.lsst.io/
    py::class_<VArrayWrapper> c(m, "VArrayWrapper");
    c.def(py::init<>())
        .def("__len__", &VArrayWrapper::size)
	.def("__iter__", [](VArrayWrapper& vec) -> VArrayIterator<diff_edit> {
	    return {vec};
        });

    py::class_<VArrayIterator<diff_edit>>(m, "VArrayIterator<diff_edit>")
        .def("__iter__", [](VArrayIterator<diff_edit>& it) -> VArrayIterator<diff_edit> { return it; })
        .def("__next__", &VArrayIterator<diff_edit>::next);

    // https://github.com/pybind/pybind11/blob/master/tests/test_enum.cpp
    py::enum_<diff_op>(m, "diff_op", py::arithmetic(), "The type of operation that an diff_edit object performs")
        .value("DIFF_MATCH", DIFF_MATCH, "Represents an unchanged part of the text")
        .value("DIFF_DELETE", DIFF_DELETE, "Represents a deletion in the text")
        .value("DIFF_INSERT", DIFF_INSERT, "Represents an insertion in the text")
        .export_values();

    py::class_<diff_edit>(m, "diff_edit")
      .def_readwrite("op", &diff_edit::op)
      .def_readwrite("off", &diff_edit::off)
      .def_readwrite("len", &diff_edit::len);

    // https://github.com/pybind/pybind11/blob/master/tests/test_exceptions.cpp
    // make a new custom exception and use it as a translation target
    static py::exception<PylcsException> ex(m, "PylcsException");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) {
                std::rethrow_exception(p);
            }
        } catch (const PylcsException &e) {
            // Set MyException as the active python error
            ex(e.what());
        }
    });


    m.def("lcs", &lcs, R"pbdoc(
        Longest common subsequence
    )pbdoc");

    m.def("lcs_of_list", &lcs_of_list, R"pbdoc(
        Longest common subsequence of list
    )pbdoc");

    m.def("lcs_matrix", &lcs_matrix, R"pbdoc(
        Longest common subsequence as a matrix
    )pbdoc");

    m.def("lcs_string", &lcs_string, R"pbdoc(
        Longest common subsequence as a string
    )pbdoc");

    m.def<VArrayWrapper(const string&, const string&) /*`diff`'s function return type followed by its param types, to help msvc out*/
	  >("diff", &diff, R"pbdoc(
        Diff two strings, returning a varray of libmba diff_edit operations needed to get from the first string to the second. To use the returned object, you can use the usual Python iterators.
    )pbdoc");

    m.def("lcs2", &lcs2, R"pbdoc(
        Longest common substring
    )pbdoc");

    m.def("lcs2_of_list", &lcs2_of_list, R"pbdoc(
        Longest common substring of list
    )pbdoc");

    m.def("levenshtein_distance", &levenshtein_distance, R"pbdoc(
        Levenshtein Distance of Two Strings
    )pbdoc");

    m.def("edit_distance", &levenshtein_distance, R"pbdoc(
        Same As levenshtein_distance(): Levenshtein Distance of Two Strings
    )pbdoc");

    m.def("levenshtein_distance_of_list", &levenshtein_distance_of_list, R"pbdoc(
        Levenshtein Distance of one string to a list of strings
    )pbdoc");

    m.def("edit_distance_of_list", &levenshtein_distance_of_list, R"pbdoc(
        Levenshtein Distance of one string to a list of strings
    )pbdoc");
}
