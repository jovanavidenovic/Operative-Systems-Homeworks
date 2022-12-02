
#!/bin/bash

function help(){
    echo Uporaba: $0 akcija parametri
    exit 0;
}

function replace_chars(){
var=-1;
for arg in "$@";
do
    if [[ $var == -1 ]]
    then
	 var=$(( $var + 1 ));
    else
    arg=${arg//[a]/ha};
    arg=${arg//[e]/he};
    arg=${arg//[i]/hi};
    arg=${arg//[o]/ho};
    arg=${arg//[u]/hu};
    echo $var: "$arg";
    var=$(($var + 1));
    fi;
    done
    exit 0;
}

function st(){
min=$1;
if [[ $min -gt $2 ]]
then min=$2;
fi;
if [[ $(($1 % $min)) == 0 ]] && [[ $(($2 % $min)) == 0 ]]
then    res=$min;
    echo $res;
    exit $(($res%256));
fi;

min=$((min/2));
for i in $(seq 2 $min); 
do
    ost1=$(($1 % $i));
    ost2=$(($2 % $i));
    if [[ $ost1 == 0 ]] && [[ $ost2 == 0 ]];
    then res=$i;
    fi;
done;
echo $res;
exit $(($res%256));
}


function check_leapyr(){
var=-1;
for arg in "$@";
do
    if [[ $var == -1 ]]
    then
         var=$(( $var + 1 ));
    else
    str="Leto $arg ni prestopno."
    (([ $(($arg % 4)) -eq 0 ] && [ $(($arg % 100)) -ne 0 ]) || [ $(($arg % 400)) -eq 0 ]) && str="Leto $arg je prestopno.";
    echo ${str};
    fi;
done;
exit 0;
}

function fib(){
for((i=$#; i > 1; i=$((i - 1))));
do
    printf "${!i}: ";  fibiter "${!i}";
done;
exit 0;
}

function fibiter(){  
	var1=0; 
	var2=1; 
        for ((ii=2; ii <= $1; ii=$((ii + 1))));
	do 
		temp=$var2; 
		var2=$(($var1+$temp)); 
		var1=$temp; 
	done; 	
	
	if [ $1 == 0 ]; 
	then var2=0; 
	fi; 
	
	echo $var2; 
}

function points(){
num_students=0;
general_sum=0;
RANDOM=42;
	while read line;
	do 
        if [[ "${line:0:1}" != "#" ]];
        then 
            IFS=' ';
            read -a arr <<< "$line";
            vpisna=${arr[0]};
            a=${arr[1]};
            b=${arr[2]};
            c=${arr[3]};
            sum_points=$(($a + $b + $c));
            lng=${#arr[@]};
            if [[ $lng -eq 5 ]];
            then 
                tip=${arr[4]};
                if  [[ tip -eq "p" ]] || [[ tip -eq "P" ]];
                then 
                    sum_points=$(( $sum_points / 2 ));
                    elif [[ ${vpisna:2:2} -eq  14 ]];
                    then 
                        sum_points=$(( $sum_points + 1 + ("$RANDOM" % 5) ));
                fi;
            elif [[ ${vpisna:2:2} -eq  14 ]];
                then sum_points=$(( $sum_points + 1 + ("$RANDOM" % 5) ));
            fi;

            if [[ $sum_points -gt 50 ]];
            then            
                sum_points=50;
            fi;
            general_sum=$(($sum_points + $general_sum));
            num_students=$(($num_students + 1));
            printf "$vpisna: $sum_points\n";
        fi;
	done;
printf "St. studentov: $num_students\n";
average_sum=$(( $general_sum / $num_students ));
printf "Povprecne tocke: $average_sum\n";

exit 0;
}



function users(){

for arg in "$@";
do 
    user_name=$arg;
    printf "$user_name: ";
    grep -cq "^$user_name" /etc/passwd;
    exists="$?";

    if [[ $exists == 0 ]];
    then
        uid=`cat /etc/passwd | grep "^$user_name" | cut -d: -f3`;
        gid=`cat /etc/passwd | grep "^$user_name" | cut -d: -f4`;

        if [[ $uid == $gid ]];
        then
            printf "enaka ";
        else
            printf "razlicna ";
        fi;

        home_dir=`cat /etc/passwd | grep "$uid" | cut -d: -f6`;
        if [[ -z "$home_dir" ]];
        then
            printf "ne-obstaja ";
        else
            printf "obstaja ";
        fi;

        num_groups=`cat /etc/group | grep -c "$user_name"`;
        num_groups=$((num_groups + 1));
        printf "$num_groups ";

        num_proc=`ps -U $user_name | wc -l`;
        printf "$num_proc\n";

    else 
        printf "err\n";
    fi;
done;

exit 0;
}


function drevo(){



    exit 0;
}

if [[ "$1" == "pomoc" ]];
    then help;
elif [[ "$1" == "hehho" ]];
    then replace_chars "$@";
elif [[ "$1" == "status" ]];
    then st "$2" "$3";
elif [[ "$1" == "leto" ]];
    then check_leapyr "$@";
elif [[ "$1" == "stej" ]];
    then 
        cat "${@:2}" | tr -d " \t" | grep -v "^#" | sed '/^[[:space:]]*$/d' | cut -d: -f2 | sort | uniq -c | sort -r | nl;
        exit 0;
elif [[ "$1" == "fib" ]];
    then fib "$@";
elif [[ "$1" == "tocke" ]];
	then points "$2";
elif [[ "$1" == "upori" ]];
    then users "${@:2}";
elif [[ "$1" == "drevo" ]];
    then drevo "$2" "${$3:=10}";
else echo "Napacna uporaba skripte!"; 
     help;
fi;

