#! /bin/bash

function print_time()
{
	start=$1
	end=$2
	start_s=`echo $start | cut -d '.' -f 1`
	start_ns=`echo $start | cut -d '.' -f 2`
	end_s=`echo $end | cut -d '.' -f 1`
	end_ns=`echo $end | cut -d '.' -f 2`
	time_micro=$(( (10#$end_s-10#$start_s)*1000000 + (10#$end_ns/1000 - 10#$start_ns/1000) ))
	time_ms=`expr $time_micro/1000  | bc `
	echo "$time_micro us"
	echo "$time_ms ms"
}


START=`date  +%s.%N`
var1=0
var2=0
for((i=1;i<=$1;i++));do
	gen=`expr $i + 100`
	var1=`expr $var1 + 1`
	tmp=`expr $i % 100`

	if [ "$var1" -ge 255 ];then
		var1=0
		var2=`expr $var2 + 1`
	fi
	
	./01 192.168.$var2.$var1 $gen eth0 10.1.111.1

	if [ "$tmp" -eq 0 ];then
                echo "the number: $i"
		FINISH=`date +%s.%N`
	        print_time $START $FINISH
        fi
done

#FINISH=`date +%s.%N`
#print_time $START $FINISH
