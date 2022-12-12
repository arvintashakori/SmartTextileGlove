from io import open
from os import path
from setuptools import find_packages, setup

here = path.abspath(path.dirname(__file__))

# get reqs
def requirements():
    list_requirements = []
    with open('requirements.txt') as f:
        for line in f:
            list_requirements.append(line.rstrip())
    return list_requirements

setup(
      name='SmartTextileGlove',
      version='1.0.0',
      description='SmartTextileGlove: Stretchable Smart Textile Gloves for Dynamic Tracking of Articulated Hands',
      url='https://github.com/arvintashakori/SmartTextileGlove/',
      author='Arvin Tashakori',
      license='All rights reserved for Arvin Tashakori 2022-2023',
      author_email='arvin@ece.ubc.ca',
      packages=find_packages(exclude=['']),
      python_requires='>=3.9',
      install_requires=requirements()
)
