#!/usr/bin/csh
/usr/bin/rsync --omit-dir-times --include='*.mid.lz4' -avL /home/mu3e/online/runs/ --address=129.129.143.120 langenegger@129.129.185.171:/data/experiment/mu3e/data/2025/raw
#/usr/bin/rsync --omit-dir-times --include='*.mid.lz4' -avL /data1/run2025/runs/ --address=129.129.143.120 langenegger@129.129.185.171:/data/experiment/mu3e/data/2025/raw
#/usr/bin/rsync --include='*.mid.*' --exclude='*' -avL -e 'ssh -b 129.129.143.120'  /home/mu3e/online/online/ langenegger@129.129.185.171:/data/experiment/mu3e/data/2025/raw >& /dev/null
 
 
