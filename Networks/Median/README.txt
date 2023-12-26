This directory contains individual median selection networks as JSON files.
The names follow the format 
	Median_<N>_<L>_<D>.json
Where
    N is the number of inputs
    L is the number of elements (CE nodes) in the network (=size)
    D is the number of layers in the network (=depth)

JSON was chosen because of its simplicity and universal availability for nearly all platforms.
Apart from the three fields above and the actual network, the file contains a symmetry flag that is set iff
the network is vertically symmetric. This property can be exploited to allow more efficient implementations.
Obviously, this symmetry is only possible for even N (technically also for the trivial case N=1).
Although the network files show one layer per line when opened in a text editor, semantically there is just a single flat list of elements.
 
The JSON specification can be found at
https://www.ecma-international.org/wp-content/uploads/ECMA-404_2nd_edition_december_2017.pdf
