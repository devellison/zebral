#include <filesystem>
#include <fstream>
#include "args.hpp"
#include "http_client.hpp"
#include "log.hpp"
#include "network.hpp"

using namespace zebral;
static const std::vector<ArgsConfigEntry> kArgTable = {{"help", '?', nullptr, "Show help"}};

static const size_t kMaxFiles = 30;

std::string gFilename;
size_t gCount = 0;

bool OnFileReceived(uint8_t* data, size_t length, const std::vector<std::string>& headers)
{
  std::string filename = gFilename;
  if (gCount > 0)
  {
    std::filesystem::path fname = filename;
    std::string new_fname =
        fname.stem().string() + "_" + std::to_string(gCount) + fname.extension().string();

    fname.replace_filename(new_fname);
    filename = fname.string();
  }
  std::cerr << "Writing: " << filename << std::endl;
  for (const auto& curHeader : headers)
  {
    std::cerr << "   " << curHeader << std::endl;
  }

  std::ofstream outfile(filename, std::ios::binary);
  outfile.write(reinterpret_cast<const char*>(data), length);

  gCount++;
  return gCount < kMaxFiles;
}

int main(int argc, char** argv)
{
  Platform p;
  Network n;

  Args args(argc, argv, kArgTable);
  if (args.has_errors())
  {
    args.display_errors();
  }

  if (args.has_flag("help") || (args.num_arguments() < 2))
  {
    args.display_help("Usage: zebral_get [OPTIONS] URL FILE USERNAME PASSWORD");
    std::cerr << std::endl;
    std::cerr << "Example: zebral_get http://10.0.0.22/image test.jpg" << std::endl;
    return 1;
  }
  std::string user, pwd;
  if (args.num_arguments() > 2) user = args.get_argument(2);
  if (args.num_arguments() > 3) pwd = args.get_argument(3);

  std::string uri          = args.get_argument(0);
  BinaryCurlClient* client = nullptr;

  gFilename = args.get_argument(1);

  int response;
  if (uri.compare(0, 4, "http") == 0)
  {
    client   = new HttpClient(user, pwd);
    response = client->Get(args.get_argument(0), OnFileReceived);
  }
  else
  {
    ZBA_ERR("Unknown client type in url! Only http is supported currently.");
    return 1;
  }

  delete client;

  if (response != 200)
  {
    std::cerr << "Got response: " << std::to_string(response) << std::endl;
    return 1;
  }
  return 0;
}