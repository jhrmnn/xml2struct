This small mex function converts an XML file into a MATLAB structure. 

Specification of the conversion:

* All attributes are ignored.
* Elements are parsed in sets of siblings with the same name (a family).
* A family can be converted either to a 1-D struct array, a 1-D cell array or a 1-D double array.
* If the first node of the first sibling from a family is an element named "real\_array\_object", this family will become either a real array or a cell vector of real arrays, depending on the number of siblings. If the first node of the first sibling is any other element (not a data node), this family will become a struct array. If it is a data node, but it cannot be converted into a double scalar, the family will become a cell array. If it can be converted, the family will become a double vector.
* The root of the document is an exception to the rule above and will always become a scalar struct. Thus it should contain elements.
* A "real\_array\_object" object has to have a structure as in the following example, which would be converted into a 2x2x1 real array.

        <real_array_object>
            <dimension>2</dimension>
            <dimension>2</dimension>
            <dimension>1</dimension>
            <vector>
                <vector>
                    <cell>0.974252514356440</cell>
                    <cell>-0.122132687560354E-08</cell>
                </vector>
                <vector>
                    <cell>-0.225459657745978</cell>
                    <cell>0.282637135081547E-09</cell>
                </vector>
            </vector>
        </real_array_object>

* A struct array family is converted such that i-th element of the struct array corresponds to i-th sibling and field names are given by the names of child elements of the siblings. Any data nodes of the siblings are ignored. Values are resolved recursively using this and following rules.
* A cell array family is converted such that i-th cell is a string equal to the value of the first node of i-th sibling. If that happens to be an element, the string will be an empty string. If a sibling has more than one node, they are all ignored.
* A double array family is converted such that i-th element is a value of the first node of i-th sibling. If that cannot be converted to double scalar, it will be set to zero.
* To summarize, the type of a family is determined by its first sibling and for the conversion to be meaningful, the siblings should have identical internal structure.
