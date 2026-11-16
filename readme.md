SIFT and RANSAC — image retrieval verification

Personal progress repository for assignment 2 of K23g, University of Athens, 2026–27.

Status: initial project description and roadmap. No assignment 2 implementation is published yet.

The complete implementation is developed separately in a private collaborative repository.
This public repository will gradually document my contributions: command-line configuration,
retrieval metrics, result formatting, reranking, experiment tooling and documentation.

The project extends a jointly authored assignment 1 by Dimitrios Mallidis and Evangelos
Tampachaniotis. Shared baseline code and the collaborator's work are not presented as individual work.
Published adaptations will keep explicit provenance and dependency information.

Planned structure:
- `include/` and `src/`: completed contributions when ready to publish.
- `report/`: selected original discussion and verified figures when available.
- `Makefile`: build instructions when the published modules form a runnable program.
- `readme.md`: current status, file descriptions and examples.

Roadmap:
- [x] Set up a separate public repository.
- [ ] Extend command-line options and validation.
- [ ] Document and extend retrieval metrics.
- [ ] Format before/after results and timing summaries.
- [ ] Implement stable reranking using the agreed verification interface.
- [ ] Add reproducible experiment scripts and verified results.

Target environment: Linux and C++20. Features are extracted with OpenCV.
Matching, normalized DLT and RANSAC belong to the team's implementation.

This repository does not currently contain a runnable verification program.
Datasets, caches and teaching materials are not included.
