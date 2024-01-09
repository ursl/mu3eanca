# Attempt to setup ndoe.js REST api to mongodb community server

`rest` contains the first working bare-bones version

## Test http into PSI
`read.js` used to test whether PSI blocks indeed everything (it does)

```
pc11740>sudo node read.js
```

```
moor>curl pc11740/
<html><body><p>This is home Page.</p></body></html>moor>curl pc11740/student
<html><body><p>This is student Page.</p></body></html>
moor>curl pc11740/admin
<html><body><p>This is admin Page.</p></body></html>
```