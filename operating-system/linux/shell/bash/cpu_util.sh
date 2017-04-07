#/bin/bash

# usage: run this command, nohup pcmant.sh & 
# author: MathxH
# email: brainfvck@foxmail.com

get_cpu_usage()
{
read cpu user nice system idle iowait irq softirq steal guest< /proc/stat

# compute active and total utilizations
cpu_active_prev=$((user+system+nice+softirq+steal))
cpu_total_prev=$((user+system+nice+softirq+steal+idle+iowait))

sleep 5

# Read /proc/stat file (for second datapoint)
read cpu user nice system idle iowait irq softirq steal guest< /proc/stat

# compute active and total utilizations
cpu_active_cur=$((user+system+nice+softirq+steal))
cpu_total_cur=$((user+system+nice+softirq+steal+idle+iowait))

# compute CPU utilization (%)
cpu_util=$((100*( cpu_active_cur-cpu_active_prev ) / (cpu_total_cur-cpu_total_prev) ))

return $cpu_util
}


while [ 1 ]
do
  get_cpu_usage
  RESULT=$?
  if [ $RESULT -ge 95 ] 
  then   
    echo "$RESULT >= 95  restarting server..."
    pkill -f java
    pkill -f tomcat
    bash /root/tomcat7/bin/startup.sh
    echo "restarting server finished"
    fi
done
