This small mex function converts an XML file into a MATLAB structure. 

Specification of the conversion:

* All element attributes are ignored.
* Elements are parsed in sets of siblings with the same name (called a family here).
* A family can be converted to different types of objects according to the following recursive rules.
    * If the first node of the first sibling from a family is a data node and it can be converted into a real scalar, the family will become a 1D real array. If it cannot be converted, the family will become a single string or a 1D cell array of strings, depending on the family size.
    * If the first node of the first sibling is an element named "dimension", this family will become either an nD real array (details below) or a 1D cell array of nD real arrays, depending on the family size. 
    * If the first node of the first sibling is any other element (not a data node), this family will become a 1D struct array (whose fields can be 1D struct arrays, etc.). This is the recursive rule.
* The root of the document is an exception and will always be resolved according to the last rule (as a struct scalar). It should contain at least one element.
* An nD real array has to have a structure as in the following example, which would be resloved into a 2x2x1 real array. The dimensionality is unlimited.
```xml
<some_array_name>
    <dimension>2</dimension>
    <dimension>2</dimension>
    <dimension>1</dimension>
    <vector>
        <vector>
            <cell>1</cell>
            <cell>-1e5</cell>
        </vector>
        <vector>
            <cell>1e-10</cell>
            <cell>0</cell>
        </vector>
    </vector>
</some_array_name>
```
