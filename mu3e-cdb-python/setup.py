"""Setup script for mu3e_cdb package."""

from setuptools import setup, find_packages
from pathlib import Path

# Read the README file for long description
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text(encoding="utf-8")

setup(
    name="mu3e-cdb",
    version="0.1.0",
    description="Python API for Mu3e Conditions Database",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="Urs Langenegger",
    author_email="urslangenegger@gmail.com", 
    url="https://github.com/ursl/mu3eanca",  # Replace with actual repository URL
    packages=find_packages(exclude=["tests", "tests.*", "examples", "examples.*"]),
    python_requires=">=3.7",
    install_requires=[
        # No required dependencies - all are optional
    ],
    extras_require={
        "rest": ["requests"],  # For REST database
        "mongo": ["pymongo"],  # For MongoDB support
        "all": ["requests", "pymongo"],  # All optional dependencies
        "dev": [
            "pytest>=6.0",
            "pytest-cov",
            "black",
            "flake8",
        ],
    },
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Physics",
        "License :: OSI Approved :: MIT License",  # Update if different license
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
    ],
    keywords="mu3e conditions database calibration physics",
    project_urls={
        "Documentation": "https://github.com/mu3e/mu3eanca",  # Replace with actual docs URL
        "Source": "https://github.com/mu3e/mu3eanca",  # Replace with actual repo URL
        "Tracker": "https://github.com/mu3e/mu3eanca/issues",  # Replace with actual issues URL
    },
    include_package_data=True,
    zip_safe=False,
)

