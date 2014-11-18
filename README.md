This small mex function converts an XML file into a MATLAB structure. 

Specification of the conversion:

* All element attributes are ignored.
* Elements are parsed in sets of siblings with the same name (called a family here).
* A family can be converted to different types of objects according to the following recursive rules.
    * If the first node of the first sibling from a family is a data node and it can be converted into a real scalar, the family will become a 1D real array. If it cannot be converted, the family will become a single string or a 1D cell array of strings, depending on the family size.
    * If the first node of the first sibling is an element named "dimension", this family will become either an nD real array (details below) or a 1D cell array of nD real arrays, depending on the family size. 
    * If the first node of the first sibling is any other element (not a data node), this family will become a 1D struct array (whose fields can be 1D struct arrays, etc.). This is the recursive rule.
* The root of the document is an exception and will always be resolved according to the last rule (as a struct scalar). It should contain at least one element.

So for example this `test.xml`

```xml
<?xml version="1.0"?>
<inception>
    <dream>
        <dream>1</dream>
        <dream>2</dream>
        <dream>3</dream>
    </dream>
    <dream>
        <dream>Dream 1</dream>
        <dream>Dream 2</dream>
        <dream>Dream 3</dream>
    </dream>
    <dream>
        <dream>Dream</dream>
    </dream>
    <another_dream>
        <dimension>2</dimension>
        <dimension>1</dimension>
        <dimension>2</dimension>
        <vector>
            <vector>
                <cell>1</cell>
                <cell>-1e5</cell>
            </vector>
        </vector>
        <vector>
            <vector>
                <cell>1e-5</cell>
                <cell>-1</cell>
            </vector>
        </vector>
    </another_dream>
    <another_dream>
        <dimension>1</dimension>
        <cell>2</cell>
    </another_dream>
</inception>
```

would be resolved as

```matlab
inception = xml2struct('test.xml')
% inception = 
%     another_dream: {2x1 cell}
%             dream: [3x1 struct]
inception.dream.dream
% ans =
%      1
%      2
%      3
% ans = 
%     'Dream 1'
%     'Dream 2'
%     'Dream 3'
% ans =
% Dream
inception.another_dream
% ans = 
%     [2x1x2 double]
%     [           2]
```
