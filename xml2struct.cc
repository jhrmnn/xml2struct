#include <string>
#include <map>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <mex.h>

using namespace std;
using namespace rapidxml;

mxArray *parse_elements(xml_node<> **elem_p, int n_elem)
{
    xml_node<> *elem = *elem_p;
    typedef map<string,int> dict;
    dict children;
    for (xml_node<> *child = elem->first_node(); child; child = child->next_sibling()) {
        children[child->name()]++;
    }
    const char **fieldnames = new const char*[children.size()];
    int i = 0;
    for (dict::iterator it = children.begin(); it != children.end(); it++) {
        fieldnames[i++] = it->first.c_str();
    }
    mxArray *data = mxCreateStructMatrix(n_elem, 1, children.size(), fieldnames);
    delete[] fieldnames;
    for (i = 0; i < n_elem; i++) {
        for (xml_node<> *child = elem->first_node(); child; child = child->next_sibling()) {
            if (child->value_size() > 0) {
                mxSetField(data, i, child->name(), mxCreateDoubleScalar(atof(child->value())));
            }
            else {
                mxSetField(data, i, child->name(), parse_elements(&child, children[child->name()]));
            }
        }
        if (i < n_elem-1) {
            elem = elem->next_sibling();
        }
    }
    *elem_p = elem;
    return data;
}

void mexFunction(int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[])
{
    char filename[200];
    mxGetString(prhs[0], filename, 200);
    file<> xml_file(filename);
    xml_document<> doc;
    doc.parse<0>(xml_file.data());
    xml_node<> *root = doc.first_node();
    plhs[0] = parse_elements(&root, 1);
}

