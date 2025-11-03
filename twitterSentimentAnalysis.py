import tweepy
from textblob import TextBlob
import csv

# Your bearer token
bearer_token = 'AAAAAAAAAAAAAAAAAAAAAL795AEAAAAAyTseEN4C6hUGNz8F9r7QvWpJbnI%3DdTTGSiraYQtib8M6C2F2HCauRsBjtutYtqDWzwWsOYH0hxBrz4'

# Initialize client
client = tweepy.Client(bearer_token=bearer_token)

# Search query
query = 'Trump -is:retweet'

# Fetch tweets
tweets = client.search_recent_tweets(query=query, max_results=10, tweet_fields=['text', 'created_at', 'author_id'])

# Create CSV file
with open('tweets_sentiment.csv', mode='w', newline='', encoding='utf-8') as file:
    writer = csv.writer(file)
    # Header row
    writer.writerow(['Tweet', 'Polarity', 'Subjectivity', 'Author_ID', 'Created_At'])
    
    if tweets.data:
        for tweet in tweets.data:
            text = tweet.text
            analysis = TextBlob(text)
            polarity = analysis.polarity
            subjectivity = analysis.subjectivity
            author_id = tweet.author_id
            created_at = tweet.created_at
            # Write row
            writer.writerow([text, polarity, subjectivity, author_id, created_at])
    else:
        print("No tweets found.")

print("CSV file created successfully!")

