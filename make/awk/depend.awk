#!/bin/awk -f 

BEGIN {FS = ":"; RS = "!";}

{
    sub(/[[:blank:]]*$/,"",$1)
    sub(/[[:blank:]]*$/,"",$2)
    sub(/[[:blank:]]*$/,"",$3)
    sub(/[[:blank:]]*$/,"",$4)
    sub(/[[:blank:]]*$/,"",$5)
    sub(/^[[:blank:]]*/,"",$1)
    sub(/^[[:blank:]]*/,"",$2)
    sub(/^[[:blank:]]*/,"",$3)
    sub(/^[[:blank:]]*/,"",$4) 
    sub(/^[[:blank:]]*/,"",$5) 

    if(1 == NR)
    {
    	root=$1
    }
    type[$1]=$2;
    file[$1]=$3;
    depend[$1]=$4;
    syslib[$1]=$5;
}

function GetArray(x,a)
{
    return split(x,a," ")
}

function Log(msg)
{
    if (debug)
        print msg > "/dev/stderr";
}

END {

    # all bins is skipped
    # if after static($), marked with '$' suffix, except already added
    # if after dynamic($^*) static(*) static2(*), marked with '*' suffix, except already added
    # if after static(^) static2(^) static2($), marked with '^' suffix, except already added

    type[root] = "proj-lib-static";
    mark[root] = "$"; # "$" "^", "*"
    result[0] = root;
    idx[root] = 0;
    syslibs[0] = "";
    i = 0;
    n = 1;
    nl = 0; #syslib
    while (i < n) {
        if (!(i in result)) {
            i++;
            continue;
        }
        item = result[i++];
        Log("visit("item") | "type[item]" | "mark[item]);
        if (type[item] == "proj-lib-static" && mark[item] == "$") {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
                Log("  "di);
                if (di in idx) {
                    Log("    del("di","mark[di]","idx[di]")");
                    delete result[idx[di]];
                    Log("    add("di",$,"n")");
                    mark[di] = "$";
                    idx[di] = n;
                    result[n++] = di;
                } else if (pack || type[di] != "proj-bin") {
                    Log("    add("di",$,"n")");
                    mark[di] = "$";
                    idx[di] = n;
                    result[n++] = di;
                } else {
                    Log("    skip("di")");
                }
            }
        } else if (type[item] == "proj-lib-dynamic" || type[item] == "proj-bin" || mark[item] == "*") {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
                Log("  "di);
                if (di in idx) {
                } else if (type[di] != "proj-bin") {
                    Log("    add("id",*,"n")");
                    mark[di] = "*";
                    idx[di] = n;
                    result[n++] = di;
                } else {
                    Log("    skip("di")");
                }
            }
            if (!pack)
                syslib[item] = "";
        } else {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
                Log("  "di);
                if (di in idx) {
                } else if (type[di] != "proj-bin") {
                    Log("    add("di",^,"n")");
                    mark[di] = "^";
                    idx[di] = n;
                    result[n++] = di;
                } else {
                    Log("    skip("di")");
                }
            }
        }
        ii = GetArray(syslib[item], deplibs);
        for (j = 1; j <= ii; ++j) {
            si = deplibs[j];
            ++nl;
            if (si in idxl) {
                Log("  del syslib("si","idxl[si]")");
                delete syslibs[idxl[si]];
            } 
            Log("  add syslib("si","nl")");
            idxl[si] = nl;
            syslibs[nl] = si;
        }
    }

    # all static, after static are include
    # all dynamic after dynamic are include with directory only, without file

    for (l in syslib) {
        Log(l"->"syslib[l]);
    }

    for (i = 1; i < n; i++) {
        if (i in result) {
            item = result[i];
            if (mark[item] == "$") {
                ii = GetArray(file[item], files);
                if (!pack) {
                    for (j = 1; j <= ii; ++j) {
                        Log(item"/"files[j]);
                        print item"/"files[j];
                    }
                } else {
                    GetArray(syslib[item], syslibs);
                    for (j = 1; j <= ii; ++j) {
                        Log(result[i]"/"files[j]":"syslibs[j]);
                        print result[i]"/"files[j]":"syslibs[j];
                    }
                }
            } else if (type[item] == "proj-lib-dynamic" && mark[item] == "*") {
                ii = GetArray(file[item], files);
                if (!pack) {
                    for (j = 1; j <= ii; ++j) {
                        #sub(/[^\/]*$/,"",files[j]);
                        Log(item"/"files[j]);
                        print item"/"files[j];
                    }
                } else {
                    GetArray(syslib[item], syslibs);
                    for (j = 1; j <= ii; ++j) {
                        Log(item"/"files[j]":"syslibs[j]);
                        print item"/"files[j]":"syslibs[j];
                    }
                }
            }
        }
    }

    if (!pack) {
        for (i = 1; i <= nl; i++) {
            if (i in syslibs) {
                Log(syslibs[i]);
                print syslibs[i];
            }
        }
    }
}
