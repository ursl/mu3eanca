# mu3e
mu3e analysis and calibration software

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
