# Use an official Python runtime as a parent image
FROM python

# Set the working directory
WORKDIR /home/app/
COPY ./* /home/app/

# Install needed packages
RUN pip install Flask
RUN pip install Redis

# Make port 80 available to the world outside of this container
EXPOSE 80

# Run app.py when the container launches
CMD python app.py
