#include <iostream>
#include <ostream>
namespace{

	// redirect outputs to another output stream.
	class redirect_outputs
{
	std::ostream& myStream;
	std::streambuf *const myBuffer;
public:
	redirect_outputs(std::ostream& lhs, std::ostream& rhs = std::cout)
		: myStream(rhs), myBuffer(myStream.rdbuf())
	{
		myStream.rdbuf(lhs.rdbuf());
	}

	~redirect_outputs() {
		myStream.rdbuf(myBuffer);
	}
};

// redirect output stream to a string.
class capture_outputs
{
	std::ostringstream myContents;
	const redirect_outputs myRedirect;
public:
	capture_outputs(std::ostream& stream = std::cout)
		: myContents(), myRedirect(myContents, stream)
	{}
	std::string contents() const
	{
		return (myContents.str());
	}
};

}