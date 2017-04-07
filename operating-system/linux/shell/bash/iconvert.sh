    #!/bin/bash
   
    #@brief:convert the encode of a bunch of files under the directory and the sub-directory
    #@Author:AlexiaChen
       
    if [ "$1" == "--help" ]; then   
        echo "Usage: `basename $0` [dir] [file_pattern] [from_code] [to_code]"   
        exit   
    fi  
       
    dir=$1   
    pattern=$2
    from_code=$3
    to_code=$4   
    echo $1  
       
    for file in `find $dir -name "$pattern"`; do   
        echo "$file"   
        iconv -f $from_code -t $to_code -o $file $file   
    done  
