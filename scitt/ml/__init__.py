"""Transformer track finding on scitt silicon-hit dumps.

One training example is one frame. Tokens are reconstructed hits.
``tid`` / ``hid`` / ``pid`` / truth position are labels only, never model input.

Run from this directory (``scitt/``, tcsh)::

    python -m ml.train --synthetic --epochs 5
    python -m ml.train --root /path/to/scitt.root --epochs 20
"""
