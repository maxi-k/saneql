# assumes that settings and variables
# from root project Makefile are included

INTEGRATIONS_PREFIX=$(PREFIX)integrations/
so_src:=integrations/sqlite-saneql.cpp integrations/duckdb-saneql.cpp
so_obj:=$(addprefix $(PREFIX),$(so_src:.cpp=.o))
so_so:=$(addprefix $(PREFIX),$(so_src:.cpp=.so))

-include $(addprefix $(PREFIX),$(so_src:.cpp=.d))

#$(CXX) $(CXXFLAGS) -c $< -o $@/
$(INTEGRATIONS_PREFIX)%.o: %.cpp
	$(checkdir)
	$(compilecpp) -fpic

$(INTEGRATIONS_PREFIX)%.so: $(INTEGRATIONS_PREFIX)%.o $(obj)
	$(checkdir)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^

integrations: $(so_so)
.PHONY: integrations
