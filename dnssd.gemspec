Gem::Specification.new do |s|
  s.name        = 'dnssd'
  s.version     = '9.9.9'
  s.summary     = "Bonjour bindings for Ruby"
  s.description = ""
  s.authors     = ['Eric Hodel', 'Aaron Patterson', 'Phil Hagelberg', 'Chad Fowler', 'Charles Mills', 'Rich Kilmer']
  s.files       = Dir.glob("{bin,lib,ext}/**/*")
  s.homepage    = 'https://github.com/jacobjennings/dnssd'
  s.extensions << "ext/dnssd/extconf.rb"
  s.add_development_dependency "rake-compiler"
end
