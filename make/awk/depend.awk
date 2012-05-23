#!/bin/awk -f 
BEGIN {FS = ":"; RS = "!";}
{
   sub(/[[:blank:]]*$/,"",$1)
   sub(/[[:blank:]]*$/,"",$2)
   sub(/[[:blank:]]*$/,"",$3)
   sub(/[[:blank:]]*$/,"",$4)
   sub(/^[[:blank:]]*/,"",$1)
   sub(/^[[:blank:]]$/,"",$2)
   sub(/^[[:blank:]]$/,"",$3)
   sub(/^[[:blank:]]$/,"",$4) 
   if(1 == NR)
   {
   	root=$1
   }

   param1 = sprintf("%s/%s",$1,$2);
   gsub(/[[:blank:]]*/,"",param1)	
   lib[$1] = param1
   
   
   if(0 < index($2,".so") || 0 < index($2,".dll"))
   {
       type[$1]=1
   }
   else if(0 < index($2,".a"))
   {
       type[$1]=0
   }
   else
   {
       type[$1]=2
   }
   
  # print $1,type[$1]  
 
   if(1 <= length($3))
   {
#      print $1"=>"$3;
      depend[$1] =$3
   }
	
	
  # print $1,type[$1]

   if(1 <= length($4))
   {
      syslib[$1] = $4
   }
}
function GetArray(x,a)
{
return split(x,a," ")
}
END{	
    result[0] = root;
    result_lib[0] = "";
    idx[root] = 0;
    i = 0;
    n = 1;
    while (i < n) {
        if (!(i in result)) {
            i++;
            continue;
        }
        item = result[i++];
#        print "visit "item;
        if (i == 1 || type[item] == 0) {
            ii = GetArray(depend[item], depends);
            for (d in depends) {
                di = depends[d];
#                print di;
                if (di in idx) {
#                    print "del "idx[di]" "di;
                    delete result[idx[di]];
#                    print "add "n" "di;
                    idx[di] = n;
                    result[n++] = di;
                } else if (type[di] < 2) {
#                    print "add "n" "di;
                    idx[di] = n;
                    result[n++] = di;
                } else {
#                    print "skip "di;
                }
            }
            ii = GetArray(syslib[item], syslibs);
            for (s in syslibs) {
                si = syslibs[s];
                if (si in libadd) {
#                    print "skip lib "si;
                } else {
#                    print "add lib "si;
                    libadd[si] = length(result_lib);
                    result_lib[length(result_lib)] = si;
                }
            }
        }
    }
    for (i = 1; i < n; i++) {
        if (i in result) {
#            print i;
            print lib[result[i]];
        }
    }
    for (i = 1; i < length(result_lib); i++) {
        print result_lib[i];
    }
}
