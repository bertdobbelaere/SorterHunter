This directory contains individual sorting networks as JSON files.
These networks are optimized for low worst case swap count.
The names follow the format 
        Sort_LS_<N>_<L>_<D>_MAX<M>.json
Where
    <N> is the number of inputs
    <L> is the number of elements (CE nodes) in the network (=size)
    <D> is the number of layers in the network (=depth)
    <M> is the worst case maximum number of swaps "MAXSWAPS" required regardless of the input 

JSON was chosen because of its simplicity and universal availability for nearly all platforms.
Apart from the fields above and the actual network, the file contains the average swap count "AVGSWAPS" for a random input and 
contains a symmetry flag that is set iff the network is vertically symmetric. This property can be exploited to allow more efficient implementations.
Obviously, this symmetry is only possible for even N (technically also for the trivial case N=1).
Although the network files show one layer per line when opened in a text editor, semantically there is just a single flat list of elements.
 
The JSON specification can be found at
https://www.ecma-international.org/wp-content/uploads/ECMA-404_2nd_edition_december_2017.pdf
