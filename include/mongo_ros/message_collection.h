/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file 
 * 
 * The MessageCollection class
 *
 * \author Bhaskara Marthi
 */

#ifndef MONGO_ROS_MESSAGE_COLLECTION_H
#define MONGO_ROS_MESSAGE_COLLECTION_H

#include <mongo_ros/query_results.h>
#include <ros/ros.h>

namespace mongo_ros
{

/// Represents a collection of ROS Messages stored in a MongoDB database.
/// Each stored message in the db has a unique id, a creation time, and
/// optional additional metadata stored as a dictionary
template <class M>
class MessageCollection
{
public:

  /// \brief Will connect to given database and collection.  Collection is
  /// created if it doesn't exist.
  /// \param db_host If provided, will be used instead of looking up the
  /// warehouse_host ros parameter.
  /// \param db_port If provided, will be used instead of looking up the
  /// warehouse_port ros parameter.
  /// \param timeout Throw a DbConnectException if can't connect within
  /// this many seconds
  MessageCollection (const std::string& db_name,
                     const std::string& collection_name,
                     const std::string& db_host="",
                     unsigned db_port=0,
                     float timeout=300.0);

  /// \brief Insert a ROS message, together with some optional metadata,
  /// into the db
  /// \throws mongo::DBException if unable to insert
  /// \param metadata Metadata to insert.  Note that a unique id field
  /// _id and a field creation_time will be autogenerated for all messages.
  ///
  /// As a secondary effect, publishes a notification message on the topic
  /// warehouse/db/collection/inserts
  void insert (const M& msg, const Metadata& metadata = Metadata());

  /// \retval Iterator range over matching messages
  /// \param query A metadata object representing a query.
  /// \param metadata_only If this is true, only retrieve the metadata
  /// (returned message objects will just be default constructed)
  typename QueryResults<M>::range_t
  queryResults (const mongo::Query& query, bool metadata_only = false,
                const std::string& sort_by = "", bool ascending=true) const;

  /// \brief Convenience function that calls queryResult and
  /// puts the results into a vector
  std::vector<typename MessageWithMetadata<M>::ConstPtr >
  pullAllResults (const mongo::Query& query, bool metadata_only = false,
                  const std::string& sort_by = "", bool ascending=true) const;

  
  /// \brief Convenience function that returns a single matching result
  /// for the query
  /// \throws NoMatchingMessageException
  typename MessageWithMetadata<M>::ConstPtr
  findOne (const Query& query, bool metadata_only = false) const;

  /// \brief Remove messages matching query
  unsigned removeMessages (const mongo::Query& query);

  /// \post Ensure that there's an index on the given field.
  /// Note that index on _id and creation_time are always created.
  MessageCollection& ensureIndex (const std::string& field);
  
  /// \brief Modify metadata
  /// Find message matching \a q and update its metadata using \a m
  /// In other words, overwrite keys in the message using \a m, but
  /// keep keys that don't occur in \a m.
  void modifyMetadata (const Query& q, const Metadata& m);

  /// \brief Count messages in collection
  unsigned count ();

  /// \brief Check if the md5 sum of the messages previously stored in 
  /// the database matches that of the compiled message
  bool md5SumMatches () const;
  
private:

  // Called by constructors
  void initialize (const std::string& db, const std::string& coll,
                   const std::string& host, unsigned port,
                   float timeout);

  const std::string ns_;
  boost::shared_ptr<mongo::DBClientConnection> conn_;
  boost::shared_ptr<mongo::GridFS> gfs_;
  bool md5sum_matches_;
  ros::NodeHandle nh_;
  ros::Publisher insertion_pub_;
};


} // namespace

#include "impl/message_collection_impl.hpp"

#endif // include guard
