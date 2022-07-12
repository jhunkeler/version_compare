# version_compare

```
usage: version_compare {{v} | {v1} {operator} {v2}}
{v} execution example:
    version_compare "1.2.3 >  1.2.3"
    0
    version_compare "1.2.3 >= 1.2.3"
    1
    version_compare "1.2.3 <  1.2.3"
    0
    version_compare "1.2.3 <= 1.2.3"
    1
    version_compare "1.2.3 != 1.2.3"
    0
    version_compare "1.2.3 =  1.2.3"
    1

{v1} {operator} {v2} execution example:
    version_compare "1.2.3" ">"  "1.2.3"
    0
    version_compare "1.2.3" ">=" "1.2.3"
    1
    version_compare "1.2.3" "<"  "1.2.3"
    0
    version_compare "1.2.3" "<=" "1.2.3"
    1
    version_compare "1.2.3" "!=" "1.2.3"
    0
    version_compare "1.2.3" "="  "1.2.3"
    1
```

## Example

```shell
#!/usr/bin/env bash

v1=1.0.0
v2=1.0.1
op='>='

result=$(version_compare "$v1 $op $v2")
if [ -z "$result" ]; then
    # operation failed
    exit 1
fi

if (( result )); then
    # operation true
else
    # operation false
fi
```