# Collection Solution Pack [![Build Status](https://travis-ci.org/Islandora/islandora_solution_pack_collection.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_solution_pack_collection)

## Introduction

Adds all required Fedora objects to allow for the creation of collections and the management of collections and child objects.
This module is required for islandora to have collection functionality.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)

**NOTE**: The collection solution pack either requires that the [resource index](https://wiki.duraspace.org/display/FEDORA37/Resource+Index) is enabled in Fedora or you have the [Islandora Solr](https://github.com/islandora/islandora_solr_search) module installed to perform its queries.

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Set the default number of collection objects per page, view style, and configuration options in Administration » Islandora » Core Collection (admin/islandora/basic_collection).

![Configuration screen 1](https://user-images.githubusercontent.com/2857697/39012304-aefb387a-43d9-11e8-8077-9146dcff2ef5.jpg)

**Display Generation** allows you to select one of the hookable backends for generating the list of objects in a collection. 

_Note_: **Solr** option only appears if you have the [Islandora Solr](https://github.com/islandora/islandora_solr_search) module installed.

![Configuration screen 2](https://user-images.githubusercontent.com/2857697/39012313-b1262678-43d9-11e8-988b-f62c0fb0d5b5.jpg)

## Documentation

Further documentation for this module is available at [our wiki](https://wiki.duraspace.org/display/ISLANDORA/Collection+Solution+Pack).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors
Current maintainers:

* [William Panting](https://github.com/willtp87)

## Development

If you would like to contribute to this module, please check out [CONTRIBUTING.md](CONTRIBUTING.md). In addition, we have helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the [Islandora.ca](http://islandora.ca) site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
