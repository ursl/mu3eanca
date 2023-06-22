# mu3eanca
MU3E ANalysis and CAlibration software

Usage:


git clone git@github.com:ursl/mu3eanca <br>
cd mu3eanca <br>

git submodule init<br>
git submodule update<br>


cd util<br>
make<br>

cd ../ana<br>
make<br>

mkdir results<br>
ln -s /to/some/place/run/data<br>
bin/runTreeReader01 -f data/mu3e_trirec_000001.root -n 10 -D results<br>


For db0/cdb1 you do NOT need util, but other various external dependencies (mariadb, mongodb, mongocxx, etc)
