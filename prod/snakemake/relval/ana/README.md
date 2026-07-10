# relval ana skeleton

Minimal C++ ROOT analysis scaffold using `ROOT::RDataFrame`.

## Build

```bash
make
```

This creates `bin/runFillHistograms`.

## Example usage

Default frames-tree multi-hist mode (books a small preset list and skips missing branches):

```bash
./bin/runFillHistograms \
  --tree frames \
  --out ana.root \
  /path/to/trirec-cosmic-twolayer.root \
  /path/to/trirec-cosmic-threelayer.root
```

Single-hist mode:

```bash
./bin/runFillHistograms \
  --single \
  --tree frames \
  --branch runId \
  --bins 200 --xmin 0 --xmax 10000 \
  --out ana.root \
  /path/to/trirec-cosmic-twolayer.root \
  /path/to/trirec-cosmic-threelayer.root
```

## Notes

- Default tree is now `frames`, matching observed trirec ROOT outputs.
- `HistBooking` demonstrates one-pass multi-hist booking with `RDataFrame`.
- Extend `HistBooking::defaultTrirecDefs()` and/or add scenario-specific booking sets.
