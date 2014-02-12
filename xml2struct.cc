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

mxArray *parse_family(vector<xml_node<>*> family)
{
    int i;

    /*
    fields = defaultdict(defaultdict(list))
    for i, sibling in enumerate(family):
        for node in sibling:
            if type(node) is element:
                fields[node.name][i].append(node)
    */
    dict_2 fields;
    for (i = 0; i < family.size(); i++) {
        for (xml_node<> *node = family[i]->first_node(); node; node = node->next_sibling()) {
            if (node->type() == node_element) {
                fields[node->name()][i].push_back(node);
            }
        }
    }

    /*
    fieldnames = fields.keys()
    data = # 1-D struct array with len(family) elements and fields <fieldnames>
    */
    const char **fieldnames = new const char*[fields.size()];
    i = 0;
    for (dict_2::iterator it = fields.begin(); it != fields.end(); it++) {
        fieldnames[i++] = it->first.c_str();
    }
    mxArray *data = mxCreateStructMatrix(family.size(), 1, fields.size(), fieldnames);
    delete[] fieldnames;

    /*
    for fieldname, cousins in fields:
        for i, siblings in cousins:
            if type(siblings[0]) is data:
                if parsed(siblings[0].value) is double:
                    value = # 1-D double array from siblings[:].value
                else:
                    value = # 1-D cell array from siblings[:].value
            else:
                value = parse_family(siblings)
            data[i].fieldname = value
    */
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
                value = parse_family(it_1->second);
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
    plhs[0] = parse_family(root);
}

