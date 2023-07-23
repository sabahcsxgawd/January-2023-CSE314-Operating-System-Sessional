#!/bin/bash


mand_args=4

if [ "$#" -lt "$mand_args" ]
then
  echo "Usage:";
  echo "./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]"
  echo ""
  echo "-v: verbose"
  echo "-noexecute: do not execute code files"
  kill -INT $$
fi

if [ "$5" == "-v" ]
then
   echo "Found $(ls ./$3 | wc -l) test files"
fi

for roll in ./$1/*.zip
do
   ttt=${roll: -11}
   ttt=${ttt%.zip} 
   unzip -qf "$roll" -d ./$1/$ttt
done

rm -rf "./$2"
mkdir -p "./$2/C"
mkdir -p "./$2/Java"
mkdir -p "./$2/Python"

declare -A fileTypes
declare -A matched
declare -A unmatched

for dir in ./$1/*
do
   if [ -d "$dir" ]
   then
      cFile=$(find "$dir" -name *.c)
      if [[ -n "$cFile" ]]
      then
         fileTypes["${dir: -7}"]=C
         matched["${dir: -7}"]=0
         unmatched["${dir: -7}"]=0
         if [ "$5" == "-v" ]
         then
            echo "Organizing files of ${dir: -7}"
         fi
         mkdir -p "./$2/C/${dir: -7}"
         cp "$cFile" "./$2/C/${dir: -7}/main.c"
         if [ "$5" != "-noexecute" ] && [ "$6" != "-noexecute" ]
         then
            if [ "$5" == "-v" ]
            then
               echo "Executing files of ${dir: -7}"
            fi
            gcc "./$2/C/${dir: -7}/main.c" -o "./$2/C/${dir: -7}/main.out"
            for i in $(ls "./$3/")
            do
               "./$2/C/${dir: -7}/main.out" < "./$3/$i" > "./$2/C/${dir: -7}/out${i:4}"
               if cmp -s "./$4/ans${i:4}" "./$2/C/${dir: -7}/out${i:4}"
               then
                  ((matched["${dir: -7}"]++))
               else
                  ((unmatched["${dir: -7}"]++))
               fi
            done
         fi        
      fi
   fi
done

for dir in ./$1/*
do
   if [ -d "$dir" ]
   then
      jFile=$(find "$dir" -name *.java)
      if [[ -n "$jFile" ]]
      then
         fileTypes["${dir: -7}"]=Java
         matched["${dir: -7}"]=0
         unmatched["${dir: -7}"]=0
         if [ "$5" == "-v" ]
         then
            echo "Organizing files of ${dir: -7}"
         fi
         mkdir -p "./$2/Java/${dir: -7}"
         cp "$jFile" "./$2/Java/${dir: -7}/Main.java"
         if [ "$5" != "-noexecute" ] && [ "$6" != "-noexecute" ]
         then
            if [ "$5" == "-v" ]
            then
               echo "Executing files of ${dir: -7}"
            fi
            javac "./$2/Java/${dir: -7}/Main.java"
            for i in $(ls "./$3/")
            do
               java -cp "./$2/Java/${dir: -7}/" "Main" < "./$3/$i" > "./$2/Java/${dir: -7}/out${i:4}"
               if cmp -s "./$4/ans${i:4}" "./$2/Java/${dir: -7}/out${i:4}"
               then
                  ((matched["${dir: -7}"]++))
               else
                  ((unmatched["${dir: -7}"]++))
               fi
            done
         fi         
      fi
   fi
done

for dir in ./$1/*
do
   if [ -d "$dir" ]
   then
      pFile=$(find "$dir" -name *.py)
      if [[ -n "$pFile" ]]
      then
         fileTypes["${dir: -7}"]=Python
         matched["${dir: -7}"]=0
         unmatched["${dir: -7}"]=0
         if [ "$5" == "-v" ]
         then
            echo "Organizing files of ${dir: -7}"
         fi
         mkdir -p "./$2/Python/${dir: -7}"
         cp "$pFile" "./$2/Python/${dir: -7}/main.py"
         if [ "$5" != "-noexecute" ] && [ "$6" != "-noexecute" ]
         then
            if [ "$5" == "-v" ]
            then
               echo "Executing files of ${dir: -7}"
            fi
            for i in $(ls "./$3/")
            do
               python3 "./$2/Python/${dir: -7}/main.py" < "./$3/$i" > "./$2/Python/${dir: -7}/out${i:4}"
               if cmp -s "./$4/ans${i:4}" "./$2/Python/${dir: -7}/out${i:4}"
               then
                  ((matched["${dir: -7}"]++))
               else
                  ((unmatched["${dir: -7}"]++))
               fi
            done
         fi
      fi
   fi
done

if [ "$5" != "-noexecute" ] && [ "$6" != "-noexecute" ]
then
   echo "student_id","type","matched","not_matched" > "./$2/result.csv"
   for roll in "${!fileTypes[@]}"
   do
      echo "$roll","${fileTypes[$roll]}","${matched[$roll]}","${unmatched[$roll]}" >> "./$2/result.csv"
   done
fi