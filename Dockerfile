FROM gcc:14-bookworm AS build
WORKDIR /src
COPY src/ src/
RUN g++ -std=c++20 -O2 -static -o /qseries src/main.cpp

FROM scratch
COPY --from=build /qseries /qseries
ENTRYPOINT ["/qseries"]
