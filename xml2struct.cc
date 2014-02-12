#include <string>
#include <map>
#include <vector>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <mex.h>

using namespace std;
using namespace rapidxml;

typedef vector<xml_node<>*> list_1;
typedef map<int,list_1> dict_1;
typedef map<string,dict_1> dict_2;

mxArray *parse_element(vector<xml_node<>*> elems)
{
    int i;
    dict_2 fields;
    for (i = 0; i < elems.size(); i++) {
        for (xml_node<> *child = elems[i]->first_node(); child; child = child->next_sibling()) {
            if (child->type() == node_element) {
                fields[child->name()][i].push_back(child);
            }
        }
    }

    const char **fieldnames = new const char*[fields.size()];
    i = 0;
    for (dict_2::iterator it = fields.begin(); it != fields.end(); it++) {
        fieldnames[i++] = it->first.c_str();
    }
    mxArray *data = mxCreateStructMatrix(elems.size(), 1, fields.size(), fieldnames);
    delete[] fieldnames;

    char *end_p;
    double d;
    double *arr;
    int dims[1];
    mxArray *value;
    for (dict_2::iterator it_2 = fields.begin(); it_2 != fields.end(); it_2++) {
        for (dict_1::iterator it_1 = it_2->second.begin(); it_1 != it_2->second.end(); it_1++) {
            if (it_1->second[0]->first_node()->type() == node_data) {
                d = strtod(it_1->second[0]->value(), &end_p);
                if (*end_p == '\0') {
                    value = mxCreateDoubleMatrix(it_1->second.size(), 1, mxREAL);
                    arr = mxGetPr(value);
                    for (i = 0; i < it_1->second.size(); i++) {
                        arr[i] = strtod(it_1->second[i]->value(), &end_p);
                    }
                } else {
                    dims[0] = it_1->second.size();
                    value = mxCreateCellArray(1, dims);
                    for (i = 0; i < it_1->second.size(); i++) {
                        mxSetCell(value, i, mxCreateString(it_1->second[i]->value()));
                    }
                }
            } else {
                value = parse_element(it_1->second);
            }
            mxSetField(data, it_1->first, it_2->first.c_str(), value);
        }
    }
    return data;
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    char filename[200];
    mxGetString(prhs[0], filename, 200);
    file<> xml_file(filename);
    xml_document<> doc;
    doc.parse<0>(xml_file.data());
    list_1 root;
    root.push_back(doc.first_node());
    plhs[0] = parse_element(root);
}

