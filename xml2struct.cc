/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "mex.h"

template<typename T> void split(T& tokens, const std::string& str, const std::string& delims, bool trimEmpty = false)
{
    size_t start = 0;
    size_t end = 0;
    while (end != std::string::npos) {
        end = str.find_first_of(delims, start);
        if (start != end || !trimEmpty) {
            tokens.push_back(typename T::value_type(str, start, (end != std::string::npos) ? end-start : end));
        }
        start = end+1;
    }
}

template<typename T> void split(T& tokens, const std::string& str)
{
    split(tokens, str, " \t\n", true);
}

inline std::string trimmed(const std::string& s)
{
    int i = 0;
    int n = s.length();
    while (isspace(s[i])) {
        i++;
    }
    while (isspace(s[n-1])) {
        n--;
    }
    return s.substr(i, n-i);
}

template<typename T> T tonum(const std::string& s);

template<> int tonum<int>(const std::string& s)
{
    return std::stoi(s);
}

template<> double tonum<double>(const std::string& s)
{
    try {
        return std::stod(s);
    } catch (std::out_of_range& e) {
        return 0.;
    }
}

template<typename T> void tonum(std::vector<T>& tokens, const std::vector<std::string>& v)
{
    for (auto& s : v) {
        tokens.push_back(tonum<T>(s));
    }
}

typedef rapidxml::xml_node<>* xmlnode;

mxArray* parse_family(std::vector<xmlnode>);
mxArray* parse_arrays(std::vector<xmlnode>);

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    assert(nlhs == 1);
    assert(nrhs == 1);
    char filename[200];
    mxGetString(prhs[0], filename, 200);
    rapidxml::file<> xml_file { filename };
    rapidxml::xml_document<> doc {};
    doc.parse<0>(xml_file.data());
    plhs[0] = parse_family(std::vector<xmlnode> { doc.first_node() });
}

mxArray* parse_family(std::vector<xmlnode> family)
{
    std::map<std::string, std::map<int, std::vector<xmlnode>>> fields {};
    for (size_t i = 0; i < family.size(); i++) {
        for (auto c = family[i]->first_node(); c; c = c->next_sibling()) {
            if (c->type() == rapidxml::node_element) {
                fields[c->name()][i].push_back(c);
            }
        }
    }

    const char** fieldnames = new const char*[fields.size()];
    bool contract = true;
    int i = 0;
    for (const auto& field_family : fields) {
        fieldnames[i++] = field_family.first.c_str();
        if (field_family.second.size() > 1) {
            contract = false;
        }
    }
    mxArray* data = mxCreateStructMatrix((contract) ? 1 : family.size(), 1, fields.size(), fieldnames);
    delete[] fieldnames;

    for (const auto& __fields_pair : fields) {
        auto& field = __fields_pair.first;
        auto& families = __fields_pair.second;
        for (const auto& __families_pair : families) {
            auto& i_sibling = __families_pair.first;
            auto& children = __families_pair.second;
            if (!children[0]->first_node()) {
                continue;
            }
            bool is_array = false;
            if (children[0]->first_attribute("type")) {
                std::string type { children[0]->first_attribute("type")->value() };
                if (type == "real" || type == "int") {
                    is_array = true;
                }
            }
            mxArray* value;
            if (is_array) {
                value = parse_arrays(children);
            } else if (children[0]->first_node()->type() == rapidxml::node_element) {
                value = parse_family(children);
            } else {
                int size = children.size();
                if (size > 1) {
                    value = mxCreateCellArray(1, &size);
                    for (i = 0; i < size; i++) {
                        mxSetCell(value, i, mxCreateString(trimmed(children[i]->value()).c_str()));
                    }
                } else {
                    value = mxCreateString(trimmed(children[0]->value()).c_str());
                }
            }
            mxSetField(data, (contract) ? 0 : i_sibling, field.c_str(), value);
        }
    }

    return data;
}

mxArray* parse_arrays(std::vector<xmlnode> arrays)
{
    int n_arrays = arrays.size();
    mxArray* data = mxCreateCellArray(1, &n_arrays);
    for (int i_array = 0; i_array < n_arrays; i_array++) {
        std::vector<int> dims {};
        if (arrays[i_array]->first_attribute("size")) {
            std::vector<std::string> tokens {};
            split(tokens, arrays[i_array]->first_attribute("size")->value());
            tonum(dims, tokens);
        } else {
            dims.push_back(1);
        }
        int n_dims = dims.size();
        mxArray* mx_array = mxCreateNumericArray(n_dims, dims.data(), mxDOUBLE_CLASS, mxREAL);
        mxSetCell(data, i_array, mx_array);
        double* arr = mxGetPr(mx_array);
        std::vector<xmlnode> node_index(n_dims);
        std::vector<int> multi_index(n_dims, 1);
        if (n_dims > 1) {
            node_index.back() = arrays[i_array]->first_node("vector");
            for (int i = n_dims-2; i > 0; i--) {
                node_index[i] = node_index[i+1]->first_node();
            }
        }
        int running_index = 0;
        while (multi_index.back() <= dims.back()) {
            auto col_str = trimmed((n_dims == 1) ? arrays[i_array]->value() : node_index[1]->value());
            std::vector<std::string> tokens {};
            split(tokens, col_str);
            std::vector<double> col {};
            tonum(col, tokens);
            for (auto& x : col) {
                arr[running_index++] = x;
            }
            if (n_dims == 1) {
                break;
            }
            multi_index[1]++;
            for (int i = 1; i < n_dims; i++) {
                if (multi_index[i] > dims[i]) {
                    if (i < n_dims-1) {
                        multi_index[i] = 1;
                        multi_index[i+1]++;
                    } else {
                        break;
                    }
                } else {
                    node_index[i] = node_index[i]->next_sibling();
                    for (int j = i-1; j >= 1; j--) {
                        node_index[j] = node_index[j+1]->first_node();
                    }
                    break;
                }
            }
        }
    }
    if (n_arrays == 1) {
        mxArray* data_cell = data;
        data = mxGetCell(data_cell, 0);
        mxSetCell(data_cell, 0, NULL);
        mxDestroyArray(data_cell);
    }
    return data;
}
