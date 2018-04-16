CXX=g++
RM=rm -f
CPPFLAGS=-fopenmp -std=c++11

SRCS=image_handler.cpp mandel_logger.cpp mandel_plotter.cpp main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

mandel: $(OBJS)
	$(CXX) $(CPPFLAGS) image_handler.o mandel_logger.o mandel_plotter.o main.o -o mandel

mandel_logger.o: mandel_logger.cpp mandel_logger.hpp
	$(CXX) $(CPPFLAGS) -c mandel_logger.cpp -o mandel_logger.o 

image_handler.o: image_handler.cpp image_handler.hpp bitmap_image.hpp window.hpp
	$(CXX) $(CPPFLAGS) -c image_handler.cpp -o image_handler.o

mandel_plotter.o: mandel_plotter.cpp mandel_plotter.hpp window.hpp mandel_logger.hpp
	$(CXX) $(CPPFLAGS) -c mandel_plotter.cpp -o mandel_plotter.o


main.o: main.cpp image_handler.hpp mandel_plotter.hpp 
	$(CXX) $(CPPFLAGS) -c main.cpp -o main.o

clean:
	$(RM) *.o mandel 
