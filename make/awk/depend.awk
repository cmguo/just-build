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

END {
    type[root] = "proj-lib-static";
    result[0] = root;
    idx[root] = 0;
    result_lib[0] = "";
    i = 0;
    n = 1;
    nl = 0; #syslib
    while (i < n) {
        if (!(i in result)) {
            i++;
            continue;
        }
        item = result[i++];
#        print "visit-"item"-";
        if (type[item] == "proj-lib-static") {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
#                print di;
                if (di in idx) {
#                    print "del-"idx[di]"-"di;
                    delete result[idx[di]];
#                    print "add-"n"-"di"-";
                    idx[di] = n;
                    result[n++] = di;
                } else if (type[di] != "proj-bin") {
#                    print "add-"n"-"di"-";
                    idx[di] = n;
                    result[n++] = di;
                } else {
#                    print "skip-"di;
                }
            }
            ii = GetArray(syslib[item], syslibs);
            for (j = 1; j <= ii; ++j) {
                si = syslibs[j];
                ++nl;
                if (si in idxl) {
#                    print "del lib "idxl[si]"-"si;
                    delete result_lib[ idxl[si]];
                } 
#                print "add lib "nl"-"si;
                idxl[si] = nl;
                result_lib[nl] = si;
            }
        } else {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
#                print di;
                if (di in idx) {
                } else if (type[di] != "proj-bin") {
#                    print "add-"n"-"di"-";
                    idx[di] = n;
                    result[n++] = di;
                    type[di] = type[di]"*";
                } else {
#                    print "skip-"di;
                }
            }
        }
    }
    for (i = 1; i < n; i++) {
        if (i in result) {
#            print i;
            item = result[i];
            if (type[item] !~ /\*/) {
                ii = GetArray(file[item], files);
                for (j = 1; j <= ii; ++j) {
                    print result[i]"/"files[j];
                }
            } else if (type[item] == "proj-lib-dynamic*") {
                ii = GetArray(file[item], files);
                sub(/[^\/]*$/,"",files[1]);
                print result[i]"/"files[1];
            }
        }
    }
    for (i = 1; i <= nl; i++) {
        if (i in result_lib) {
#            print i;
            print result_lib[i];
        }
    }
}
