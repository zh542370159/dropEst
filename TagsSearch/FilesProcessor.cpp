#include "FilesProcessor.h"

#include <sstream>
#include <string>
#include <stdexcept>

FilesProcessor::FilesProcessor(const std::string &r1_filename, const std::string &r2_filename,
							   const std::string &base_name, long max_reads)
		: base_name(base_name)
		, max_reads(max_reads)
		, current_file_reads_written(0)
		, out_file_index(1)
{
	this->r1_file.open(r1_filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!this->r1_file)
	{
		std::ostringstream ss;
		ss << "can't open R1 file \"" << r1_filename << "\"";
		throw std::runtime_error(ss.str());
	}

	r2_file.open(r2_filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!this->r2_file)
	{
		std::ostringstream ss;
		ss << "can't open R2 file \"" << r2_filename << "\"";
		throw std::runtime_error(ss.str());
	}

	if (boost::ends_with(r1_filename, ".gz") || boost::ends_with(r1_filename, ".gzip"))
	{
		this->r1_fs.push(boost::iostreams::gzip_decompressor());
	}
	this->r1_fs.push(r1_file);

	if (boost::ends_with(r2_filename, ".gz") || boost::ends_with(r2_filename, ".gzip"))
	{
		this->r2_fs.push(boost::iostreams::gzip_decompressor());
	}
	this->r2_fs.push(r2_file);

	std::string out_file_name = this->get_out_filename();
	this->out_file.open(out_file_name.c_str(), std::ios_base::out | std::ios_base::binary);

	this->out_zip.push(boost::iostreams::gzip_compressor());
	this->out_zip.push(this->out_file);
}

std::string FilesProcessor::get_out_filename()
{
	std::stringstream ss;
	ss << this->base_name << "." << this->out_file_index << ".fastq.gz";
	return ss.str();
}

void FilesProcessor::increase_out_file()
{
	this->out_file_index++;
	std::string out_file_name = this->get_out_filename();

	this->out_zip.pop();
	this->out_file.close();
	this->out_file.open(out_file_name.c_str(), std::ios_base::out | std::ios_base::binary);
	this->out_zip.push(this->out_file);
}

bool FilesProcessor::get_r1_line(std::string &out)
{
	return std::getline(this->r1_fs, out).good();
}

bool FilesProcessor::get_r2_line(std::string &out)
{
	return std::getline(this->r2_fs, out).good();
}

void FilesProcessor::close()
{
	this->out_zip.pop();
	this->out_file.close();
}

bool FilesProcessor::write(const std::string &text)
{
	bool result = false;
	if (this->current_file_reads_written > this->max_reads)
	{
		this->increase_out_file();

		this->out_file_index++;
		current_file_reads_written = 0;
		result = true;
	}

	this->out_zip << text << std::flush;
	this->current_file_reads_written++;

	return result;
}