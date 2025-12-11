"""Setup script for mu3e_cdb package."""

from setuptools import setup, find_packages

setup(
    name="mu3e-cdb",
    version="0.1.0",
    description="Python API for Mu3e Conditions Database",
    author="Mu3e Collaboration",
    packages=find_packages(),
    install_requires=[
        # No required dependencies - all are optional
    ],
    extras_require={
        "rest": ["requests"],  # For REST database
        "mongo": ["pymongo"],  # For MongoDB support
        "all": ["requests", "pymongo"],  # All optional dependencies
    },
    python_requires=">=3.7",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
)

