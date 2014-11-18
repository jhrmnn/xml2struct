This small mex function converts an XML file into a MATLAB structure. 

Specification of the conversion:

* All attributes are ignored.
* Elements are parsed in sets of siblings with the same name (a family).
* A family can be converted to different types of objects.
    * If the first node of the first sibling from a family is a data node and it can be converted into a real scalar, the family will become a 1D real array. If it cannot be converted, the family will become a single string or a 1D cell array of strings, depending on the family size.
    * If the first node of the first sibling is an element named "dimension", this family will become either a real array or a 1D cell array of real arrays, depending on the number of siblings.
    * If the first node of the first sibling is any other element (not a data node), this family will become a 1D struct array.
    * The root of the document is an exception to the rule above and will always become a scalar struct. Thus it should contain elements.
* A nD real array object has to have a structure as in the following example, which would be converted into a 2x2x1 real array named `some_array`.

```xml
<some_array>
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
</some_array>
```
