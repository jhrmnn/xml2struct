#include <string>
#include <map>
#include <vector>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "mex.h"

using namespace std;
using namespace rapidxml;

typedef vector<xml_node<>*> list_p_node;
typedef map<int,list_p_node> dict_i_to_list;
typedef map<string,dict_i_to_list> dict_str_to_dict;

mxArray *parse_arrays(list_p_node arrays)
{
    int n_arrays = arrays.size();
    mxArray *data = mxCreateCellArray(1, &n_arrays);
    for (int i_array = 0; i_array < n_arrays; i_array++) {
        vector<int> dims;
        xml_node<> *node;
        for (node = arrays[i_array]->first_node(); node; node = node->next_sibling()) {
            if (strcmp(node->name(), "dimension") != 0) {
                break;
            }
            dims.push_back(atoi(node->value()));
        }
        int n_dims = dims.size();
        int* dims_arr = new int[n_dims];
        for (int i = 0; i < n_dims; i++) {
            dims_arr[i] = dims[i];
        }
        mxArray *mx_array = mxCreateNumericArray(n_dims, dims_arr, mxDOUBLE_CLASS, mxREAL);
        delete[] dims_arr;
        mxSetCell(data, i_array, mx_array);
        double *arr = mxGetPr(mx_array);
        list_p_node node_index(n_dims);
        node_index.back() = node;
        for (int i = n_dims-2; i >= 0; i--) {
            node_index[i] = node_index[i+1]->first_node();
        }
        vector<int> multi_index(n_dims, 1);
        int running_index = 0;
        while (true) {
            arr[running_index] = atof(node_index[0]->value());
            running_index++;
            multi_index[0]++;
            for (int i = 0; i < n_dims; i++) {
                if (multi_index[i] <= dims[i]) {
                    node_index[i] = node_index[i]->next_sibling();
                    for (int j = i-1; j >= 0; j--) {
                        node_index[j] = node_index[j+1]->first_node();
                    }
                    break;
                } else if (i < n_dims-1) {
                    multi_index[i] = 1;
                    multi_index[i+1]++;
                }
            }
            if (multi_index.back() > dims.back()) {
                break;
            }
        }
    }
    if (n_arrays == 1) {
        mxArray *data_cell = data;
        data = mxGetCell(data_cell, 0);
        mxSetCell(data_cell, 0, NULL);
        mxDestroyArray(data_cell);
    }
    return data;
}

mxArray *parse_family(list_p_node family)
{
    int i;

    /*
    fields = defaultdict(defaultdict(list))
    for i, sibling in enumerate(family):
        for node in sibling:
            if type(node) is element:
                fields[node.name][i].append(node)
    */
    dict_str_to_dict fields;
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
    for (dict_str_to_dict::iterator it = fields.begin(); it != fields.end(); it++) {
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
    mxArray *value;
    for (dict_str_to_dict::iterator it_str_dict = fields.begin(); it_str_dict != fields.end(); it_str_dict++) {
        for (dict_i_to_list::iterator it_i_list = it_str_dict->second.begin(); it_i_list != it_str_dict->second.end(); it_i_list++) {
            if (it_i_list->second[0]->first_node() == 0)
                continue;
            if (it_i_list->second[0]->first_node()->type() == node_data) {
                d = strtod(it_i_list->second[0]->value(), &end_p);
                if (*end_p == '\0') {
                    value = mxCreateDoubleMatrix(it_i_list->second.size(), 1, mxREAL);
                    arr = mxGetPr(value);
                    for (i = 0; i < it_i_list->second.size(); i++) {
                        arr[i] = strtod(it_i_list->second[i]->value(), &end_p);
                    }
                } else {
                    int size = it_i_list->second.size();
                        value = mxCreateCellArray(1, &size);
                        for (i = 0; i < it_i_list->second.size(); i++) {
                            mxSetCell(value, i, mxCreateString(it_i_list->second[i]->value()));
                    }
                }
            } else if (strcmp(it_i_list->second[0]->first_node()->name(), "dimension") == 0) {
                value = parse_arrays(it_i_list->second);
            } else {
                value = parse_family(it_i_list->second);
            }
            mxSetField(data, it_i_list->first, it_str_dict->first.c_str(), value);
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
    list_p_node root;
    root.push_back(doc.first_node());
    plhs[0] = parse_family(root);
}

